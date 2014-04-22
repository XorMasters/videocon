#include <iostream>

#include <cpprest/json.h>

#include "VideoconSignaling.h"

using namespace utility;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;

bool VideoconSignaling::SendOffer(webrtc::SessionDescriptionInterface* offer) {
	std::cout << "Sending offer through signaling" << std::endl;

	_sendSessionDesc(_modeString, "offer", offer);
	return true;
}

bool VideoconSignaling::SendAnswer(webrtc::SessionDescriptionInterface* answer) {
	std::cout << "Sending answer through signaling" << std::endl;

	_sendSessionDesc(_modeString, "answer", answer);
	return true;
}

bool VideoconSignaling::SendCandidate(const webrtc::IceCandidateInterface* candidate) {
	std::cout << "Sending candidate through signaling" << std::endl;

	std::string candidateStr;
	candidate->ToString(&candidateStr);

	std::cout << "sdp_mid: " << candidate->sdp_mid() << std::endl;
	std::cout << "sdp_mline_index: " << candidate->sdp_mline_index() << std::endl;
	std::cout << "sdp: " << candidateStr << std::endl;

	web::json::value candidateJson;
	candidateJson["sdp_mid"] = web::json::value::string(candidate->sdp_mid());
	candidateJson["sdp_mline_index"] = web::json::value::number(candidate->sdp_mline_index());
	candidateJson["sdp"] = web::json::value::string(candidateStr);

	_postKeyValue( _modeString + "/candidates", "candidate", candidateJson.serialize());

}

void VideoconSignaling::registerObserver(ISignalingObserver* observer) {
	if(observer == NULL) {
		std::cerr << "Registering NULL Observer" << std::endl;
		return;
	}

	auto found = std::find(_observers.begin(), _observers.end(), observer);
	if(found != _observers.end()) {
		std::cerr << "Reregistering Observer that has already been registered" << std::endl;
		return;
	}

	_observers.push_back(observer);
}

void VideoconSignaling::unregisterObserver(ISignalingObserver* observer) {
	if(observer == NULL) {
		std::cerr << "Unregistering NULL observer" << std::endl;
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
	std::cout << "Initializing Firebase Observer for Event-Stream" << std::endl;

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
		std::cout << "Get description Response code: " << response.status_code() << std::endl;
		if( response.status_code() == status_codes::OK) {
			response.extract_json().then([=](web::json::value value){
				if(value.is_null()) {
					std::cout << "No " << desc << " found yet" << std::endl;
				} else {
					std::cout << desc << ": " << value.serialize() << std::endl;
					*done = true;
					web::json::value description = value[desc];
					std::cout << desc << ": " << description.as_string() << std::endl;
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
			std::cout << "Response code: " << response.status_code() << std::endl;

			std::stringstream bodyStream;
			body.serialize(bodyStream);
			std::cout << "JSON Body: " << bodyStream.str() << std::endl;
		});
	});

}

void VideoconSignaling::_doRestCall(const std::string& nameSpace, http_request& request,
	                                http_client& client, 
	                                std::function<void (http_response)> handleResponse) {
	if(request.request_uri().is_empty()) {
		uri_builder builder = _buildUri(nameSpace);
		request.set_request_uri(builder.to_uri());

		std::cout << "Request URI: " + builder.to_string() << std::endl;
	}

	client.request(request).then(handleResponse);
}

void VideoconSignaling::_notifyOfferObservers(webrtc::SessionDescriptionInterface* offer) {
	std::cout << "Notifying all offer observers" << std::endl;
	for(auto i = _observers.begin(); i != _observers.end(); ++i ) {
		(*i)->OnReceivedOffer(offer);
	}
}

void VideoconSignaling::_notifyAnswerObservers(webrtc::SessionDescriptionInterface* answer) {
	std::cout << "Notifying all answer observers" << std::endl;
	for(auto i = _observers.begin(); i != _observers.end(); ++i ) {
		(*i)->OnReceivedAnswer(answer);
	}
}

void VideoconSignaling::VideoconSignaling::_notifyCandidateObservers(webrtc::IceCandidateInterface* candidate) {
	std::cout << "Notifying all ice candidate observers" << std::endl;
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

