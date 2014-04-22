#ifndef __VIDEOCON_SIGNALLING_H__
#define __VIDEOCON_SIGNALLING_H__

#include <cpprest/http_client.h>

#include "talk/app/webrtc/peerconnectionfactory.h"

class ISignaling {
public:	
	enum SignalingMode { eCaller, eCallee };

public:
	virtual ~ISignaling() {}

public:
	virtual bool SendOffer(webrtc::SessionDescriptionInterface* offer) = 0;
	virtual bool SendAnswer(webrtc::SessionDescriptionInterface* answer) = 0;
	virtual bool SendCandidate(const webrtc::IceCandidateInterface* candidate) = 0;

	virtual bool checkAnswer() = 0;
	virtual bool checkOffer() = 0;
};

class ISignalingObserver {
public:
	virtual ~ISignalingObserver() {}

public:
	virtual void OnReceivedOffer(webrtc::SessionDescriptionInterface* offer) = 0;
	virtual void OnReceivedAnswer(webrtc::SessionDescriptionInterface* answer) = 0;
	virtual void OnReceivedCandidate(webrtc::IceCandidateInterface* candidate) = 0;
};

class VideoconSignaling : public ISignaling {
public:
	VideoconSignaling(const std::string name, SignalingMode mode) : _sessionName(name), 
		_mode(mode), _observerClient(_appServer) { _initObserver(); }
	virtual ~VideoconSignaling() {}

public:
	void registerObserver(ISignalingObserver* observer);
	void unregisterObserver(ISignalingObserver* observer);

public:
	SignalingMode getMode() const { return _mode; }

public:
	// Implementation  of ISignaling interface
	virtual bool SendOffer(webrtc::SessionDescriptionInterface*);
	virtual bool SendAnswer(webrtc::SessionDescriptionInterface* answer);
	virtual bool SendCandidate(const webrtc::IceCandidateInterface* candidate);

	virtual bool checkOffer();
	virtual bool checkAnswer();

private:
	web::http::uri _buildUri(const std::string& nameSpace);
	void _sendSessionDesc(const std::string& nameSpace, 
		                  const std::string& type, 
		                  webrtc::SessionDescriptionInterface* desc);
	void _saveKeyValue(const std::string& method,
		               const std::string& nameSpace, 
		               const std::string& key, 
		               const std::string& value);
	void _postKeyValue(const std::string& nameSpace, 
	                   const std::string& key, 
	                   const std::string& value) { 
		_saveKeyValue(web::http::methods::POST, nameSpace, key, value); 
	}

	void _putKeyValue(const std::string& nameSpace, 
	                  const std::string& key, 
	                  const std::string& value) { 
		_saveKeyValue(web::http::methods::PUT, nameSpace, key, value);
	}

	void _initObserver();
	void _doEventStream(std::function<void (web::http::http_response)> handleResponse);
	void _doRestCall(const std::string& nameSpace, web::http::http_request& request,
	                 web::http::client::http_client& client, 
	                 std::function<void (web::http::http_response)> handleResponse);
	void _notifyOfferObservers(webrtc::SessionDescriptionInterface* offer);
	void _notifyAnswerObservers(webrtc::SessionDescriptionInterface* answer);
	void _notifyCandidateObservers(webrtc::IceCandidateInterface* candidate);
	bool _checkDescription(std::string desc, std::string mode, bool* done, std::function<void(webrtc::SessionDescriptionInterface*)> notify);
	void _getKeyValue(const std::string& key,
	                  std::function<void (web::http::http_response)> handleResponse);
	void _extractCandidates(const web::json::value& candidates);

private:
	std::string _appServer = "https://fiery-fire-2022.firebaseIO.com";
	std::string _dbName = "videocon";
	std::string _callee = "callee";
	std::string _caller = "caller";
	std::string _sessionName;
	std::string _modeString;
	SignalingMode _mode;
	std::vector<ISignalingObserver*> _observers;
	web::http::client::http_client _observerClient;
	bool _offered = false;
	bool _answered = false;
};

#endif // __VIDEOCON_SIGNALLING_H__
