#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/app/webrtc/mediastreaminterface.h"
#include "VideoconSignaling.h"

class VideoconPeerConnectionObserver : public webrtc::PeerConnectionObserver {
public:
	VideoconPeerConnectionObserver(VideoconSignaling& s, webrtc::VideoRendererInterface& r) : 
		_signaling(s), _renderer(r) {}
	virtual ~VideoconPeerConnectionObserver() {}
	
public:
	// webrtc::PeerConnecitonObserver interface
	virtual void OnStateChange(StateType state_changed);
	virtual void OnAddStream(webrtc::MediaStreamInterface* stream);
	virtual void OnRemoveStream(webrtc::MediaStreamInterface* stream);
	virtual void OnRenegotiationNeeded();
	virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);
	virtual void OnError();

private:
	VideoconSignaling& _signaling;
	webrtc::VideoRendererInterface& _renderer;
};

class VideoconSetSessionDescriptionObserver : public webrtc::SetSessionDescriptionObserver {
public:
	VideoconSetSessionDescriptionObserver() {}
	virtual ~VideoconSetSessionDescriptionObserver() {}

public: 
	virtual void OnSuccess();
    virtual void OnFailure(const std::string& error);

public:
	virtual int AddRef() { return 0; }
	virtual int Release() { return 0; }
};

class VideoconCreateSessionDescriptionObserver: public webrtc::CreateSessionDescriptionObserver {
private:
	typedef talk_base::scoped_refptr<webrtc::PeerConnectionInterface> connection_ptr;

public:
	VideoconCreateSessionDescriptionObserver(connection_ptr con, VideoconSignaling& s) : _connection(con), _signaling(s), _refCount(0) {}
	virtual ~VideoconCreateSessionDescriptionObserver() {}

public:
	virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc);
  	virtual void OnFailure(const std::string& error);

public:
	virtual int AddRef() { return 0; }
	virtual int Release() { return 0; }
	
private:
	connection_ptr _connection;
	VideoconSignaling& _signaling;
	VideoconSetSessionDescriptionObserver _observer;

	uint32_t _refCount;
};

class VideoconSignalingObserver : public ISignalingObserver {
private:
	typedef talk_base::scoped_refptr<webrtc::PeerConnectionInterface> connection_ptr;

public:
	VideoconSignalingObserver(connection_ptr connection) : _connection(connection) {}
	virtual ~VideoconSignalingObserver() {}

public:
	virtual void OnReceivedOffer(webrtc::SessionDescriptionInterface* offer);
	virtual void OnReceivedAnswer(webrtc::SessionDescriptionInterface* answer);
	virtual void OnReceivedCandidate(webrtc::IceCandidateInterface* candidate);

private:
	connection_ptr _connection;
	VideoconSetSessionDescriptionObserver _observer;
};