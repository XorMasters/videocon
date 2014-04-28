#include <cstdlib>

#define LINUX
#define LOGGING 1

//#define HAVE_GTK

#include "talk/app/webrtc/peerconnectionfactory.h"
#include "talk/app/webrtc/videosourceinterface.h"
#include "talk/base/ssladapter.h"
#include "talk/base/logging.h"
#include "talk/base/pathutils.h"
#include "talk/base/stream.h"
#include "talk/media/devices/videorendererfactory.h"
#include "talk/media/devices/devicemanager.h"
#include "talk/media/webrtc/webrtcvideocapturer.h"

#include "VideoconObservers.h"
#include "VideoconSignaling.h"
#include "AsciiVideoRenderer.h"

typedef talk_base::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory_ptr;
typedef talk_base::scoped_refptr<webrtc::PeerConnectionInterface> connection_ptr;
typedef talk_base::scoped_refptr<webrtc::MediaStreamInterface> local_media_stream_ptr;
typedef talk_base::scoped_refptr<webrtc::VideoSourceInterface> video_source_ptr;
typedef talk_base::scoped_refptr<webrtc::VideoTrackInterface> video_track_ptr;
typedef talk_base::scoped_refptr<webrtc::AudioSourceInterface> audio_source_ptr;
typedef talk_base::scoped_refptr<webrtc::AudioTrackInterface> audio_track_ptr;

int main(int argc, char* argv[]) {
	ISignaling::SignalingMode mode = (argc > 1 && std::string("-r") == argv[1]) ? ISignaling::eCaller : ISignaling::eCallee;

	talk_base::LogMessage::LogToDebug(talk_base::LogMessage::NO_LOGGING);
	
	std::string log = std::string("videocon_calle") + (mode == ISignaling::eCallee ? "e.log" : "r.log"); 
	talk_base::FileStream* logStream = talk_base::Filesystem::OpenFile(talk_base::Pathname(log), "w+");
	talk_base::LogMessage::LogToStream(logStream, talk_base::LS_VERBOSE);

	LOG(LS_INFO) << "Initializing SSL";
	talk_base::InitializeSSL();

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

	LOG(LS_INFO) << "Created peer connection";

	cricket::DeviceManagerInterface* devManager = cricket::DeviceManagerFactory::Create();
	
	LOG(LS_INFO) << "Initing device manager";
	if(!devManager->Init()) {
		LOG(LS_ERROR) << "Unable to init device manager. exiting!";
		return EXIT_FAILURE;	
	}
	
	LOG(LS_INFO) << "Device manager inited";
	
	std::vector<cricket::Device> capDevices;
	devManager->GetVideoCaptureDevices(&capDevices);
	for(auto i = capDevices.begin(); i != capDevices.end(); ++i) {
		LOG(LS_INFO) << "Video Capture device: " << i->name << ", " << i->id;
	}

	cricket::Device video;
	if(!devManager->GetVideoCaptureDevice("", &video)) {
		LOG(LS_ERROR) << "Unable to get defaule video capture device";
		return EXIT_FAILURE;
	}
	
	LOG(LS_INFO) << "Got Video Device: " << video.name << ", " << video.id;
	
	cricket::VideoCapturer* capturer = devManager->CreateVideoCapturer(video);
	if(!capturer) {
		LOG(LS_ERROR) << "Unable to create video capturer";
		return EXIT_FAILURE;
	}
	
	LOG(LS_INFO) << "Video capturer created";

	video_source_ptr source = factory->CreateVideoSource(capturer, NULL);
	
	LOG(LS_INFO) << "Created video source";
	
	video_track_ptr videoTrack = factory->CreateVideoTrack("test-video-track", source);

	LOG(LS_INFO) << "Video track created";
	
	AsciiVideoRenderer renderer(std::cout);
	videoTrack->AddRenderer(&renderer);
	
	LOG(LS_INFO) << "Video renderer created";

	cricket::Device audioIn;
	if(!devManager->GetAudioInputDevice("", &audioIn)) {
		LOG(LS_ERROR) << "Unable to get defaule audio capture device";
		return EXIT_FAILURE;		
	}

	cricket::Device audioOut;
	if(!devManager->GetAudioOutputDevice("", &audioOut)) {
		LOG(LS_ERROR) << "Unable to get defaule audio capture device";
		return EXIT_FAILURE;		
	}


	LOG(LS_INFO) << "Got Audio Input Device: " << audioIn.name << ", " << audioIn.id;
	LOG(LS_INFO) << "Got Audio Output Device: " << audioOut.name << ", " << audioOut.id;

	audio_source_ptr audioSource = factory->CreateAudioSource(NULL);

	LOG(LS_INFO) << "Created audio source";

	audio_track_ptr audioTrack = factory->CreateAudioTrack("test-audio-track", audioSource);

	LOG(LS_INFO) << "Audio track created";

	if(!localMediaStream->AddTrack(videoTrack.get())) {
		LOG(LS_ERROR) << "Unable to add video track to local media stream";
		return EXIT_FAILURE;
	}

	if(!localMediaStream->AddTrack(audioTrack.get())) {
		LOG(LS_ERROR) << "Unable to add video track to local media stream";
		return EXIT_FAILURE;
	}

	LOG(LS_INFO) << "Audio tracks added to stream";

	if(!connection->AddStream(localMediaStream.get(), NULL)) {
		LOG(LS_ERROR) << "Unable to add stream to peer connection";
		return EXIT_FAILURE;
	}

	LOG(LS_INFO) << "Local media stream added to peer connection";

	if(signaling.getMode() == ISignaling::eCallee) {
		while(!signaling.checkOffer()) {
			LOG(LS_INFO) << "### Check for an offer... ###";
			::sleep(2);
		};

		LOG(LS_INFO) << "Found offer";
	}

	if(signaling.getMode() == ISignaling::eCaller) {
		connection->CreateOffer(&createSessionObserver, NULL);
		while(!signaling.checkAnswer()) {
			LOG(LS_INFO) << "### Check for an answer... ###";
			::sleep(2);
		}
		LOG(LS_INFO) << "Found answer";
	} else {
		connection->CreateAnswer(&createSessionObserver, NULL);
	}

	::sleep(100);	

	talk_base::CleanupSSL();

	return EXIT_SUCCESS;
}