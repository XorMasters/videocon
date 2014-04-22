#include <iostream>

#include "VideoconObservers.h"

void VideoconPeerConnectionObserver::OnStateChange(StateType state_changed) {
	std::cout << ">>> State changed to: " << state_changed << std::endl;
}

void VideoconPeerConnectionObserver::OnAddStream(webrtc::MediaStreamInterface* stream) {
	std::cout << ">>> Stream added" << std::endl;
}

void VideoconPeerConnectionObserver::OnRemoveStream(webrtc::MediaStreamInterface* stream) {
	std::cout << ">>> Stream removeed" << std::endl;
}

void VideoconPeerConnectionObserver::OnRenegotiationNeeded() {
	std::cout << ">>> Negotiation needed" << std::endl;
}

void VideoconPeerConnectionObserver::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
	std::string candStr;
	candidate->ToString(&candStr);

	_signaling.SendCandidate(candidate);

	std::cout << ">>> Ice candidate received: " <<  candStr;
}

void VideoconPeerConnectionObserver::OnError() {
	std::cout << ">>> Error" << std::endl;
}

void VideoconSetSessionDescriptionObserver::OnSuccess() {
	std::cout << ">>> Succusfully added session description" << std::endl;
}

void VideoconSetSessionDescriptionObserver::OnFailure(const std::string& error) {
	std::cout << ">>> Failed to add session description" << std::endl;
}

void VideoconCreateSessionDescriptionObserver::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
	std::string offer;
	desc->ToString(&offer);

	std::cout << ">>> Successfully created session description interface: " << offer << std::endl;
	
	_connection->SetLocalDescription(&_observer, desc);

	if(_signaling.getMode() == ISignaling::eCaller) {
		_signaling.SendOffer(desc);
	} else if(_signaling.getMode() == ISignaling::eCallee) {
		_signaling.SendAnswer(desc);
	} 

}

void VideoconCreateSessionDescriptionObserver::OnFailure(const std::string& error) {
	std::cout << ">>> Failed to create session description interface" << std::endl;
}

void VideoconSignalingObserver::OnReceivedOffer(webrtc::SessionDescriptionInterface* offer) {
	std::cout << "Received an offer" << std::endl;

	_connection->SetRemoteDescription(&_observer, offer);
}

void VideoconSignalingObserver::OnReceivedAnswer(webrtc::SessionDescriptionInterface* answer) {
	std::cout << "Received an answer" << std::endl;

	_connection->SetRemoteDescription(&_observer, answer);
}

void VideoconSignalingObserver::OnReceivedCandidate(webrtc::IceCandidateInterface* candidate) {
	std::cout << "Adding ICE candidate received through signaling" << std::endl;

	_connection->AddIceCandidate(candidate);
}
