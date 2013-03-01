/*
 * Deriverd from OpenKinect examples
 */

/* thanks to Yoda---- from IRC */
class MyFreenectDevice : public Freenect::FreenectDevice {
public:
	MyFreenectDevice(freenect_context *_ctx, int _index)
	: Freenect::FreenectDevice(_ctx, _index), m_buffer_depth(640*480*sizeof(uint16_t)),m_buffer_video(freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB).bytes), mm_depth(2048), m_depth_normalized(2048), m_new_rgb_frame(false), m_new_depth_frame(false)
	{
		float min=100000000;
		float max=-100000000;
		
		printf("Depth classical : \n");
		for( unsigned int i = 0 ; i < 2048 ; i++) {
			mm_depth[i] = this->rawDepthToMeters(i);
			if (mm_depth[i]<min) {
				min=mm_depth[i];
			}
			if (mm_depth[i]>max) {
				max=mm_depth[i];
			}
			printf(" %f",(float)mm_depth[i]);
		}
		printf("\nfound min=%f max=%f.\n",min,max);
		float szconvert=1.0/((-1.0*min)+max);
		printf("szconvert %f",szconvert);
		printf("\n\nDepth normalize: \n");
		for( unsigned int i = 0 ; i < 2048 ; i++) {
			m_depth_normalized[i] = (this->rawDepthToMeters(i)-min)*szconvert;
			printf(" %f",(float)m_depth_normalized[i]);
		}
		normalized=false;
	}
	//~MyFreenectDevice(){}
	// Do not call directly even in child
	void VideoCallback(void* _rgb, uint32_t timestamp) {
		Mtx::ScopedLock lock(m_rgb_mutex);
		uint8_t* rgb = static_cast<uint8_t*>(_rgb);
		std::copy(rgb, rgb+getVideoBufferSize(), m_buffer_video.begin());
		m_new_rgb_frame = true;
	};
	// Do not call directly even in child
	void DepthCallback(void* _depth, uint32_t timestamp) {
		Mtx::ScopedLock lock(m_depth_mutex);
		uint16_t* depth = static_cast<uint16_t*>(_depth);
		if (this->normalized) {
			for( unsigned int i = 0 ; i < 640*480 ; ++i) {
				m_buffer_depth[i]=mm_depth[depth[i]]*1.0/200.0;
				//m_buffer_depth[i]=m_depth_normalized[depth[i]];
			}
		}else {
			for( unsigned int i = 0 ; i < 640*480 ; ++i) {
				m_buffer_depth[i]=mm_depth[depth[i]];
			}
		}
		m_new_depth_frame = true;
	}
	// converts 11bit raw depth data from kinect to depth in mm
	inline float rawDepthToMeters(int depthValue) {
		if (depthValue < 2047) {
			const float k1 = 1.1863;
			const float k2 = 2842.5;
			const float k3 = 0.1236;
			return k3 * tanf(depthValue/k2 + k1);
		}
		return 0.0f;
	}
	bool getRGB8bit(std::vector<uint8_t> &buffer) {
		Mtx::ScopedLock lock(m_rgb_mutex);
		if (!m_new_rgb_frame)
			return false;
		buffer.swap(m_buffer_video);
		m_new_rgb_frame = false;
		return true;
	}
	
	bool getDepth16bit(std::vector<half> &buffer) {
		Mtx::ScopedLock lock(m_depth_mutex);
		if (!m_new_depth_frame)
			return false;
		buffer.swap(m_buffer_depth);
		m_new_depth_frame = false;
		return true;
	}
	void depthNormalized(){this->normalized=true;};
	void depthInMM(){this->normalized=false;};
private:
	bool normalized; 
	//will store only raw data from kinect
	std::vector<half> m_buffer_depth;
	std::vector<uint8_t> m_buffer_video;
	Mtx m_rgb_mutex;
	Mtx m_depth_mutex;
	bool m_new_rgb_frame;
	bool m_new_depth_frame;
	std::vector<half>mm_depth;
	std::vector<half>m_depth_normalized;
};