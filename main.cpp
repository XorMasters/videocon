#include <iostream>
#include <cstdlib>

//#define LINUX
//#define HAVE_GTK

#include "talk/app/webrtc/peerconnectionfactory.h"
#include "talk/app/webrtc/videosourceinterface.h"
#include "talk/base/ssladapter.h"
#include "talk/media/devices/videorendererfactory.h"
#include "talk/media/devices/devicemanager.h"
#include "talk/media/webrtc/webrtcvideocapturer.h"

#include "VideoconObservers.h"
#include "VideoconSignaling.h"

typedef talk_base::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory_ptr;
typedef talk_base::scoped_refptr<webrtc::PeerConnectionInterface> connection_ptr;
typedef talk_base::scoped_refptr<webrtc::MediaStreamInterface> local_media_stream_ptr;
typedef talk_base::scoped_refptr<webrtc::VideoSourceInterface> video_source_ptr;
typedef talk_base::scoped_refptr<webrtc::VideoTrackInterface> video_track_ptr;
typedef talk_base::scoped_refptr<webrtc::AudioSourceInterface> audio_source_ptr;
typedef talk_base::scoped_refptr<webrtc::AudioTrackInterface> audio_track_ptr;

int main(int argc, char* argv[]) {
	std::cout << "Initializing" << std::endl;
	talk_base::InitializeSSL();

	ISignaling::SignalingMode mode = (argc >= 1 && std::string("-r") == argv[1]) ? ISignaling::eCaller : ISignaling::eCallee;

	VideoconSignaling signaling("Ascii-Cam", mode);
	VideoconPeerConnectionObserver observer(signaling);

	factory_ptr factory = webrtc::CreatePeerConnectionFactory();
	local_media_stream_ptr localMediaStream = factory->CreateLocalMediaStream("test-stream");
	
	webrtc::PeerConnectionInterface::IceServers servers;
  	webrtc::PeerConnectionInterface::IceServer server;
  	server.uri = "stun:stun.l.google.com:19302";
  	servers.push_back(server);

	connection_ptr connection = factory->CreatePeerConnection(servers, NULL, NULL, &observer);
	VideoconCreateSessionDescriptionObserver createSessionObserver(connection, signaling);
	VideoconSignalingObserver signalingObserver(connection);

	signaling.registerObserver(&signalingObserver);

	std::cout << "Created peer connection" << std::endl;

	cricket::DeviceManagerInterface* devManager = cricket::DeviceManagerFactory::Create();
	
	std::cout << "Initing device manager" << std::endl;
	if(!devManager->Init()) {
		std::cerr << "Unable to init device manager. exiting!" << std::endl;
		return EXIT_FAILURE;	
	}
	
	std::cout << "Device manager inited" << std::endl;
	
	// cricket::Device video;
	// if(!devManager->GetVideoCaptureDevice("", &video)) {
	// 	std::cerr << "Unable to get defaule video capture device" << std::endl;
	// 	return EXIT_FAILURE;
	// }
	
	// std::cout << "Got Video Device: " << video.name << ", " << video.id << std::endl;
	
	// cricket::VideoCapturer* capturer = devManager->CreateVideoCapturer(video);
	// if(!capturer) {
	// 	std::cerr << "Unable to create video capturer" << std::endl;
	// 	return EXIT_FAILURE;
	// }
	
	// std::cout << "Video capturer created" << std::endl;

	// video_source_ptr source = factory->CreateVideoSource(capturer, NULL);
	
	// std::cout << "Created video source" << std::endl;
	
	// video_track_ptr videoTrack = factory->CreateVideoTrack("test-video-track", source);

	// std::cout << "Video track created" << std::endl;
	
	// webrtc::VideoRendererInterface* renderer = cricket::VideoRendererFactory::CreateGuiVideoRenderer(0, 0);
	// if(!renderer) {
	// 	std::cerr << "Unable to create video renderer" << std::endl;
	// } else {
	// 	std::cout << "Video renderer created" << std::endl;
	// 	videoTrack->AddRenderer(renderer);
	// }

	// std::vector<cricket::Device> inDevices;
	// devManager->GetAudioInputDevices(&inDevices);
	// for(auto i = inDevices.begin(); i != inDevices.end(); ++i) {
	// 	std::cout << "Input audio device: " << i->name << ", " << i->id << std::endl;
	// }

	// std::vector<cricket::Device> outDevices;
	// devManager->GetAudioOutputDevices(&outDevices);
	// for(auto i = outDevices.begin(); i != outDevices.end(); ++i) {
	// 	std::cout << "Output audio device: " << i->name << ", " << i->id << std::endl;
	// }
	
	cricket::Device audioIn;
	if(!devManager->GetAudioInputDevice("", &audioIn)) {
		std::cerr << "Unable to get defaule audio capture device" << std::endl;
		return EXIT_FAILURE;		
	}

	cricket::Device audioOut;
	if(!devManager->GetAudioOutputDevice("", &audioOut)) {
		std::cerr << "Unable to get defaule audio capture device" << std::endl;
		return EXIT_FAILURE;		
	}


	std::cout << "Got Audio Input Device: " << audioIn.name << ", " << audioIn.id << std::endl;
	std::cout << "Got Audio Output Device: " << audioOut.name << ", " << audioOut.id << std::endl;

	audio_source_ptr audioSource = factory->CreateAudioSource(NULL);

	std::cout << "Created audio source" << std::endl;

	audio_track_ptr audioTrack = factory->CreateAudioTrack("test-audio-track", audioSource);

	std::cout << "Audio track created" << std::endl;

	// if(!localMediaStream->AddTrack(videoTrack.get())) {
	// 	std::cerr << "Unable to add video track to local media stream" << std::endl;
	// 	return EXIT_FAILURE;
	// }

	if(!localMediaStream->AddTrack(audioTrack.get())) {
		std::cerr << "Unable to add video track to local media stream" << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Audio tracks added to stream" << std::endl;

	if(!connection->AddStream(localMediaStream.get(), NULL)) {
		std::cerr << "Unable to add stream to peer connection" << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Local media stream added to peer connection" << std::endl;

	if(signaling.getMode() == ISignaling::eCallee) {
		while(!signaling.checkOffer()) {
			std::cout << "### Check for an offer... ###" << std::endl;
			::sleep(2);
		};

		std::cout << "Found offer" << std::endl;
	}

	if(signaling.getMode() == ISignaling::eCaller) {
		connection->CreateOffer(&createSessionObserver, NULL);
		while(!signaling.checkAnswer()) {
			std::cout << "### Check for an answer... ###" << std::endl;
			::sleep(2);
		}
		std::cout << "Found answer" << std::endl;
	} else {
		connection->CreateAnswer(&createSessionObserver, NULL);
	}

	::sleep(100);	

	talk_base::CleanupSSL();

	return EXIT_SUCCESS;
}