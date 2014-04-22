INC += ../trunk ../casablanca/Release/include

LIB += libcrssl libjingle_peerconnection libjingle_p2p libjingle_media 
LIB += libusrsctplib libvideo_capture_module libacm2 libaudio_coding_module   
LIB += libCNG libG711 libG722 libiLBC libiSAC libPCM16B libNetEq4 libwebrtc_opus 
LIB += libopus libmedia_file libvideo_engine_core libwebrtc_video_coding  
LIB += libwebrtc_i420 libvideo_render_module libvideo_coding_utility  
LIB += libwebrtc_vp8 libvpx_asm_offsets_vp8 libvpx libvpx_intrinsics_mmx 
LIB += libvpx_intrinsics_sse2 libvpx_intrinsics_ssse3 libvpx libpaced_sender
LIB += librbe_components libremote_bitrate_estimator libbitrate_controller 
LIB += libvideo_processing libvideo_processing_sse2 libcommon_video 
LIB += libvoice_engine libaudio_device libNetEq libmedia_file 
LIB += libaudio_conference_mixer libaudio_processing libaudioproc_debug_proto 
LIB += librtp_rtcp libwebrtc_utility libprotobuf_lite libaudio_processing_sse2
LIB += libcommon_audio libcommon_audio_sse2 libsystem_wrappers libjingle_sound 
LIB += libjingle libsrtp libcrssl libpthread libdl librt libnss3 libyuv 
LIB += libjpeg_turbo libXv libX11 libXext libexpat libcasablanca libcommon_utilities

DEF += POSIX HAVE_WEBRTC_VIDEO

FRAMEWORK += Foundation CoreServices Carbon IOKit Security AppKit QTKit 
FRAMEWORK += CoreVideo CoreAudio AudioToolbox OpenGL

LIBDIR += ../casablanca/Release/build.release/Binaries
LIBDIR += ../trunk/out/Debug/obj/talk ../trunk/out/Debug/obj/webrtc
LIBDIR += ../trunk/out/Debug/obj/webrtc/common_audio 
LIBDIR += ../trunk/out/Debug/obj/webrtc/common_video
LIBDIR += ../trunk/out/Debug/obj/webrtc/modules 
LIBDIR += ../trunk/out/Debug/obj/webrtc/system_wrappers/source
LIBDIR += ../trunk/out/Debug/obj/webrtc/tools 
LIBDIR += ../trunk/out/Debug/obj/webrtc/video
LIBDIR += ../trunk/out/Debug/obj/webrtc/video_engine 
LIBDIR += ../trunk/out/Debug/obj/webrtc/voice_engine
LIBDIR += ../trunk/out/Debug/obj/third_party/libyuv 
LIBDIR += ../trunk/out/Debug/obj/third_party/libvpx
LIBDIR += ../trunk/out/Debug/obj/net/third_party/nss  
LIBDIR += ../trunk/out/Debug/obj/third_party/protobuf
LIBDIR += ../trunk/out/Debug/obj/third_party/opus 
LIBDIR += ../trunk/out/Debug/obj/webrtc/modules/video_coding/codecs/vp8
LIBDIR += ../trunk/out/Debug/obj/third_party/icu 
LIBDIR += ../trunk/out/Debug/obj/webrtc/modules/remote_bitrate_estimator
LIBDIR += ../trunk/out/Debug 
LIBDIR += ../trunk/out/Debug/obj/webrtc/modules/video_coding/utility
LIBDIR += ../trunk/out/Debug/obj/third_party/usrsctp 
LIBDIR += ../trunk/out/Debug/obj/third_party/libsrtp
LIBDIR += ../trunk/out/Debug/obj/third_party/libjpeg_turbo

CXXFLAGS += $(patsubst %,-I%,$(INC))
CXXFLAGS += $(patsubst %,-D%,$(DEF))
CXXFLAGS += -std=c++11

#CXXFLAGS += -arch i386 -std=gnu++11
#CXXFLAGS += -mmacosx-version-min=10.6 
#CXXFLAGS += -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk

LDFLAGS += $(patsubst %,-L%,$(LIBDIR))
LDFLAGS += $(patsubst lib%,-l%,$(LIB))
#LDFLAGS += -arch i386 -std=gnu++11
#LDFLAGS += $(patsubst %,-framework %,$(FRAMEWORK))
#LDFLAGS += -Wl,-search_paths_first -Wl,-pie 
#-mmacosx-version-min=10.6 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk

videocon: VideoconObservers.o VideoconSignaling.o main.o
	$(CXX) -std=c++11 -o $@ $^ $(LDFLAGS)
	
		