#include "VideoWriter.h"

#include "../LibCore/InfoRecorder.h"
#include "../LibCore/TimeTool.h"
#define ENABLE_WRITER_LOG

namespace cg{

	HANDLE VideoWriter::syncMutex;
	bool VideoWriter::syncReset = false;
	struct timeval VideoWriter::syncTv;

	int VideoWriter::writeToFile(FILE * file, AVPacket * pkt){
		size_t ret = fwrite(pkt->data, 1, pkt->size, file);
#ifdef ENABLE_WRITER_LOG
		cg::core::infoRecorder->logTrace("[VideoWriter]: write %d data to file.\n", ret);
#endif
		return 0;
	}

	int VideoWriter::sendPacket(int channelId, AVPacket * pkt, int64_t encoderPts){

		if(isChanged){
			// the coder is changed
			encodeEnd = GetTickCount();
			cg::core::infoRecorder->logError("intra-migration(ms): %d ", encodeEnd - encodeStart);
			isChanged = false;
		}

		if(!ctx){
			cg::core::infoRecorder->logTrace("[VideoWriter]: NULL rtsp context.\n");
			return -1;
		}
		return sendPacket(channelId, ctx, pkt, encoderPts);
	}

	// for VideoWriter
	int VideoWriter::sendPacket(int channelId, rtsp::RTSPContext * rtsp, AVPacket * pkt, int64_t encoderPts){
		
#ifdef ENABLE_WRITER_LOG
		cg::core::infoRecorder->logTrace("[VideoWriter]: send packet, rtsp: %p enable rtsp:%s.\n", rtsp, rtsp->enableGen ? "true" : "false");
#endif
		
		// write the encoded frame.

		int ioLen = 0;
		uint8_t * iobuf = NULL;
		//DebugBreak();
		if(enableWriteToFile && m_fOutput){
			writeToFile(m_fOutput, pkt);
		}

		if(!rtsp->enableGen){
			return 0;
		}

		if(rtsp == NULL){
			cg::core::infoRecorder->logError("[VideoWriter]; senc packet get NULL RTSP context.\n");
			return -1;
		}
		if(rtsp->state != cg::rtsp::SERVER_STATE_PLAYING){
			cg::core::infoRecorder->logError("[VideoWriter]: server state is not SERVER_STATE_PLAY, is %d!\n", rtsp->state);

		}
		if(rtsp->fmtCtx[channelId] == NULL){
			cg::core::infoRecorder->logError("[VideoWriter]: channel %d fmtCtx is NULL.\n", channelId);
			return 0;
		}
		AVRational rq, aq;
		aq.num = rtsp->encoder[channelId]->time_base.num;
		aq.den = rtsp->encoder[channelId]->time_base.den;

		rq.den = 90000;
		rq.num = 1;
		//DebugBreak();
		if(encoderPts != (int64_t)AV_NOPTS_VALUE){
			//cg::core::infoRecorder->logError("[VideoWriter]: send packet, encoder pts: %lld, encoder time_base(num:%d, den:%d), stream time_base(num:%d, den:%d), tmp rq:(num:%d, den:%d).\n", encoderPts, rtsp->encoder[channelId]->time_base.num, rtsp->encoder[channelId]->time_base.den, rtsp->stream[channelId]->time_base.num, rtsp->stream[channelId]->time_base.den, rq.num, rq.den);
			//DebugBreak();
			//static int64_t t_pts = 0;
			//rtsp->stream[channelId]->time_base.num =1;
			//rtsp->stream[channelId]->time_base.den = 90000;
			
			pkt->pts = av_rescale_q(encoderPts, aq, rq); //rtsp->stream[channelId]->time_base);
			//pkt->pts = av_rescale_q(++t_pts, rtsp->encoder[channelId]->time_base, rtsp->stream[channelId]->time_base);

		}
		// write the pkt data
		if(ffio_open_dyn_packet_buf(&rtsp->fmtCtx[channelId]->pb, rtsp->mtu) < 0){
			cg::core::infoRecorder->logError("[VideoWriter: %d buffer allocation failed\n", channelId);
			return -1;
		}

#ifdef MULTI_SOURCE_SUPPORT
		// add the header to the packet
		static int a = 0;
		unsigned char t = a % 255;

		pkt->size += 2;   // add the frame index and the frame tag
		//infoRecorder->logError("[X264Encoder]: original buf addr:0x%p, cur:0x%p.\n", pkt->data, nalbuf_a - 2);
		pkt->data = pkt->data - 2;

		*pkt->data = 0;
		*(pkt->data + 1) = 0;
		*(pkt->data + 2) = 0;
		*(pkt->data + 3) = 1;
		// set the source id
		*(pkt->data + 4) = ctx->getId();
		*(pkt->data + 5) = (tags++) % 255;

		a++;

#endif  // MULTI_SOURCE_SUPPORT
		if(av_write_frame(rtsp->fmtCtx[channelId], pkt) != 0){
			cg::core::infoRecorder->logError("[VideoWriter]: %d RTSP write failed. fmtctx:%p\n", channelId, rtsp->fmtCtx[channelId]);
			return -1;
		}

		ioLen = avio_close_dyn_buf(rtsp->fmtCtx[channelId]->pb, &iobuf);
		if(rtsp->lowerTransport[channelId] == RTSP_LOWER_TRANSPORT_TCP){
			if(rtsp->rtspWriteBinData(channelId, iobuf, ioLen) < 0)
			{
				av_free(iobuf);
				cg::core::infoRecorder->logTrace("[VideoWriter]: %d rtsp write failed.\n", channelId);
				return -1;
			}
		}
		else{

			if(rtsp->rtpWriteBinData(channelId, iobuf, ioLen) < 0)
			{
				av_free(iobuf);
				cg::core::infoRecorder->logTrace("[VideoWriter]: %d RTP write failed.\n", channelId);
				return -1;
			}
		}
		av_free(iobuf);
		return 0;
	}

	int VideoWriter::ptsSynchronize(int sampleRate){
		struct timeval timev;
		long long us = 0;
		int ret = 0;

		WaitForSingleObject(this->syncMutex, INFINITE);
		if (this->syncReset){
			cg::core::getTimeOfDay(&syncTv, NULL);
			syncReset = false;
			ReleaseMutex(this->syncMutex);
			return 0;
		}
		cg::core::getTimeOfDay(&timev, NULL);
		us = cg::core::tvdiff_us(&timev, &syncTv);
		ReleaseMutex(this->syncMutex);
		ret = (int)(0.000001 * us * sampleRate);
		return ret > 0 ? ret : 0;
	}

	// print the information of the writer
	void VideoWriter::print(){
		cg::core::infoRecorder->logTrace("[VideoWriter]: info, rtsp:%p, file: %p.\n", ctx, m_fOutput);
	}

	int64_t VideoWriter::updataPts(int64_t _basePts, float fps /*= 30.0f*/){
		if(basePts == -1LL){
			basePts = _basePts;
			ptsSync = ptsSynchronize(fps);
			newPts = ptsSync;
		}else{
			newPts = ptsSync + _basePts - basePts;
		}

		return (pts = newPts > pts ? newPts : pts + 1);

		if(newPts > pts){
			pts = newPts;
		}
		else
			pts ++;

		return pts;
	}


}
