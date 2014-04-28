
#define LOGGING 1

#include "talk/base/logging.h"

#include "VideoconObservers.h"

typedef talk_base::scoped_refptr<webrtc::VideoTrackInterface> video_track_ptr;

void VideoconPeerConnectionObserver::OnStateChange(StateType state_changed) {
	LOG(LS_INFO) << ">>> State changed to: " << state_changed;
}

void VideoconPeerConnectionObserver::OnAddStream(webrtc::MediaStreamInterface* stream) {
	LOG(LS_INFO) << ">>> Stream added";

	video_track_ptr track = stream->FindVideoTrack("test-video-track");
	if(track.get() != NULL) {
		LOG(LS_INFO) << "Found video track. Adding renderer";
		track->AddRenderer(&_renderer);
	}
}

void VideoconPeerConnectionObserver::OnRemoveStream(webrtc::MediaStreamInterface* stream) {
	LOG(LS_INFO) << ">>> Stream removeed";
}

void VideoconPeerConnectionObserver::OnRenegotiationNeeded() {
	LOG(LS_INFO) << ">>> Negotiation needed";
}

void VideoconPeerConnectionObserver::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
	std::string candStr;
	candidate->ToString(&candStr);

	_signaling.SendCandidate(candidate);

	LOG(LS_INFO) << ">>> Ice candidate received: " <<  candStr;
}

void VideoconPeerConnectionObserver::OnError() {
	LOG(LS_INFO) << ">>> Error";
}

void VideoconSetSessionDescriptionObserver::OnSuccess() {
	LOG(LS_INFO) << ">>> Succusfully added session description";
}

void VideoconSetSessionDescriptionObserver::OnFailure(const std::string& error) {
	LOG(LS_INFO) << ">>> Failed to add session description";
}

void VideoconCreateSessionDescriptionObserver::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
	std::string offer;
	desc->ToString(&offer);

	LOG(LS_INFO) << ">>> Successfully created session description interface: " << offer;
	
	_connection->SetLocalDescription(&_observer, desc);

	if(_signaling.getMode() == ISignaling::eCaller) {
		_signaling.SendOffer(desc);
	} else if(_signaling.getMode() == ISignaling::eCallee) {
		_signaling.SendAnswer(desc);
	} 

}

void VideoconCreateSessionDescriptionObserver::OnFailure(const std::string& error) {
	LOG(LS_INFO) << ">>> Failed to create session description interface";
}

void VideoconSignalingObserver::OnReceivedOffer(webrtc::SessionDescriptionInterface* offer) {
	LOG(LS_INFO) << "Received an offer";

	_connection->SetRemoteDescription(&_observer, offer);
}

void VideoconSignalingObserver::OnReceivedAnswer(webrtc::SessionDescriptionInterface* answer) {
	LOG(LS_INFO) << "Received an answer";

	_connection->SetRemoteDescription(&_observer, answer);
}

void VideoconSignalingObserver::OnReceivedCandidate(webrtc::IceCandidateInterface* candidate) {
	LOG(LS_INFO) << "Adding ICE candidate received through signaling";

	_connection->AddIceCandidate(candidate);
}
