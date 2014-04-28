
#define LOGGING 1
#include "talk/base/logging.h"

#include <cpprest/json.h>

#include "VideoconSignaling.h"

using namespace utility;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;

bool VideoconSignaling::SendOffer(webrtc::SessionDescriptionInterface* offer) {
	LOG(LS_INFO) << "Sending offer through signaling";

	_sendSessionDesc(_modeString, "offer", offer);
	return true;
}

bool VideoconSignaling::SendAnswer(webrtc::SessionDescriptionInterface* answer) {
	LOG(LS_INFO) << "Sending answer through signaling";

	_sendSessionDesc(_modeString, "answer", answer);
	return true;
}

bool VideoconSignaling::SendCandidate(const webrtc::IceCandidateInterface* candidate) {
	LOG(LS_INFO) << "Sending candidate through signaling";

	std::string candidateStr;
	candidate->ToString(&candidateStr);

	LOG(LS_INFO) << "sdp_mid: " << candidate->sdp_mid();
	LOG(LS_INFO) << "sdp_mline_index: " << candidate->sdp_mline_index();
	LOG(LS_INFO) << "sdp: " << candidateStr;

	web::json::value candidateJson;
	candidateJson["sdp_mid"] = web::json::value::string(candidate->sdp_mid());
	candidateJson["sdp_mline_index"] = web::json::value::number(candidate->sdp_mline_index());
	candidateJson["sdp"] = web::json::value::string(candidateStr);

	_postKeyValue( _modeString + "/candidates", "candidate", candidateJson.serialize());

}

void VideoconSignaling::registerObserver(ISignalingObserver* observer) {
	if(observer == NULL) {
		LOG(LS_ERROR) << "Registering NULL Observer";
		return;
	}

	auto found = std::find(_observers.begin(), _observers.end(), observer);
	if(found != _observers.end()) {
		LOG(LS_ERROR) << "Reregistering Observer that has already been registered";
		return;
	}

	_observers.push_back(observer);
}

void VideoconSignaling::unregisterObserver(ISignalingObserver* observer) {
	if(observer == NULL) {
		LOG(LS_ERROR) << "Unregistering NULL observer";
		return;
	}

	_observers.erase(std::remove(_observers.begin(), _observers.end(), observer));
}

uri VideoconSignaling::_buildUri(const std::string& nameSpace) {
	uri_builder builder(_dbName);
	builder.append_path(_sessionName);
	if(nameSpace.length() > 0) {
		builder.append_path(nameSpace);
	}
	builder.append_path(".json");

	return builder.to_uri();
}

void VideoconSignaling::_initObserver() {
	LOG(LS_INFO) << "Initializing Firebase Observer for Event-Stream";

	_modeString = _mode == eCallee ? _callee : _caller;
}

bool VideoconSignaling::checkAnswer() {
	return _answered ? true : _checkDescription("answer",_callee, &_answered,
		                                        [=](webrtc::SessionDescriptionInterface* inter){ _notifyAnswerObservers(inter); });
}

bool VideoconSignaling::checkOffer() {
	return _offered ? true :_checkDescription("offer", _caller, &_offered,
		                                        [=](webrtc::SessionDescriptionInterface* inter){ _notifyOfferObservers(inter); });

}

bool VideoconSignaling::_checkDescription(std::string desc, std::string mode, bool* done, std::function<void(webrtc::SessionDescriptionInterface*)> notify) {
	_getKeyValue(mode, [=](http_response response){
		LOG(LS_INFO) << "Get description Response code: " << response.status_code();
		if( response.status_code() == status_codes::OK) {
			response.extract_json().then([=](web::json::value value){
				if(value.is_null()) {
					LOG(LS_INFO) << "No " << desc << " found yet";
				} else {
					LOG(LS_INFO) << desc << ": " << value.serialize();
					*done = true;
					web::json::value description = value[desc];
					LOG(LS_INFO) << desc << ": " << description.as_string();
					notify(webrtc::CreateSessionDescription(desc, description.as_string()));
					_extractCandidates(value["candidates"]);
				}
			});
		}
	});

	return false;
}

void VideoconSignaling::_doEventStream(std::function<void (http_response)> handleResponse) {
	http_request events(methods::GET);

	const std::string& mode = _mode == eCallee ? _caller : _callee;
	_doRestCall(mode, events, _observerClient, handleResponse);
}

void VideoconSignaling::_sendSessionDesc(const std::string& nameSpace, 
		                                 const std::string& type, 
		                                 webrtc::SessionDescriptionInterface* desc) {
	std::string descStr;
	desc->ToString(&descStr);

	_putKeyValue(nameSpace, type, descStr);	
}

void VideoconSignaling::_getKeyValue(const std::string& key,
	                                 std::function<void (http_response)> handleResponse) {

	http_client client(_appServer);
	http_request get(methods::GET);

	_doRestCall(key, get, client, handleResponse);
}

void VideoconSignaling::_saveKeyValue(const std::string& method,
	                                  const std::string& nameSpace, 
	                                  const std::string& key, 
	                                  const std::string& value) {
	http_client client(_appServer);
	http_request save(method);

	web::json::value json;
	json[key] = web::json::value::string(value);
	save.set_body(json);

	_doRestCall(nameSpace, save, client, [=](http_response response){
		response.extract_json().then([=](web::json::value body) {
			LOG(LS_INFO) << "Response code: " << response.status_code();

			std::stringstream bodyStream;
			body.serialize(bodyStream);
			LOG(LS_INFO) << "JSON Body: " << bodyStream.str();
		});
	});

}

void VideoconSignaling::_doRestCall(const std::string& nameSpace, http_request& request,
	                                http_client& client, 
	                                std::function<void (http_response)> handleResponse) {
	if(request.request_uri().is_empty()) {
		uri_builder builder = _buildUri(nameSpace);
		request.set_request_uri(builder.to_uri());

		LOG(LS_INFO) << "Request URI: " + builder.to_string();
	}

	client.request(request).then(handleResponse);
}

void VideoconSignaling::_notifyOfferObservers(webrtc::SessionDescriptionInterface* offer) {
	LOG(LS_INFO) << "Notifying all offer observers";
	for(auto i = _observers.begin(); i != _observers.end(); ++i ) {
		(*i)->OnReceivedOffer(offer);
	}
}

void VideoconSignaling::_notifyAnswerObservers(webrtc::SessionDescriptionInterface* answer) {
	LOG(LS_INFO) << "Notifying all answer observers";
	for(auto i = _observers.begin(); i != _observers.end(); ++i ) {
		(*i)->OnReceivedAnswer(answer);
	}
}

void VideoconSignaling::VideoconSignaling::_notifyCandidateObservers(webrtc::IceCandidateInterface* candidate) {
	LOG(LS_INFO) << "Notifying all ice candidate observers";
	for(auto i = _observers.begin(); i != _observers.end(); ++i ) {
		(*i)->OnReceivedCandidate(candidate);
	}
}

void VideoconSignaling::_extractCandidates(const web::json::value& candidates) {
	for(auto i = candidates.as_object().begin(); i != candidates.as_object().end(); i++) {
		auto candidate = web::json::value::parse(i->second.at("candidate").as_string());
		auto candidateInterface = webrtc::CreateIceCandidate(candidate["sdp_mid"].as_string(),
			                                                 candidate["sdp_mline_index"].as_number().to_int32(),
			                                                 candidate["sdp"].as_string());
		_notifyCandidateObservers(candidateInterface);
	}
}

