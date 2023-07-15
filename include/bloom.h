#pragma once
namespace gpr5300
{
	struct bloomMip
	{
		glm::vec2 size;
		glm::ivec2 intSize;
		unsigned int texture;
	};

	class bloomFBO
	{
	public:
		bloomFBO();
		~bloomFBO();
		bool Init(unsigned int windowWidth, unsigned int windowHeight, unsigned int mipChainLength);
		void Destroy();
		void BindForWriting();
		const std::vector<bloomMip>& MipChain() const;

	private:
		bool mInit;
		unsigned int mFBO;
		std::vector<bloomMip> mMipChain;
	};

	bloomFBO::bloomFBO() : mInit(false) {}
	bloomFBO::~bloomFBO() {}

	bool bloomFBO::Init(unsigned int windowWidth, unsigned int windowHeight, unsigned int mipChainLength)
	{
		if (mInit) return true;

		glGenFramebuffers(1, &mFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

		glm::vec2 mipSize((float)windowWidth, (float)windowHeight);
		glm::ivec2 mipIntSize((int)windowWidth, (int)windowHeight);
		// Safety check
		if (windowWidth > (unsigned int)INT_MAX || windowHeight > (unsigned int)INT_MAX)
		{
			std::cerr << "Window size conversion overflow - cannot build bloom FBO!" << std::endl;
			return false;
		}

		for (GLuint i = 0; i < mipChainLength; i++)
		{
			bloomMip mip;

			mipSize *= 0.5f;
			mipIntSize /= 2;
			mip.size = mipSize;
			mip.intSize = mipIntSize;

			glGenTextures(1, &mip.texture);
			glBindTexture(GL_TEXTURE_2D, mip.texture);
			// we are downscaling an HDR color buffer, so we need a float texture format
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F,
				(int)mipSize.x, (int)mipSize.y,
				0, GL_RGB, GL_FLOAT, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			std::cout << "Created bloom mip " << mipIntSize.x << 'x' << mipIntSize.y << std::endl;
			mMipChain.emplace_back(mip);
		}

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, mMipChain[0].texture, 0);

		// setup attachments
		unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, attachments);

		// check completion status
		int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("gbuffer FBO error, status: 0x%x\n", status);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			return false;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		mInit = true;
		return true;
	}

	void bloomFBO::Destroy()
	{
		for (int i = 0; i < (int)mMipChain.size(); i++)
		{
			glDeleteTextures(1, &mMipChain[i].texture);
			mMipChain[i].texture = 0;
		}
		glDeleteFramebuffers(1, &mFBO);
		mFBO = 0;
		mInit = false;
	}

	void bloomFBO::BindForWriting()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
	}

	const std::vector<bloomMip>& bloomFBO::MipChain() const
	{
		return mMipChain;
	}



	class BloomRenderer
	{
	public:
		BloomRenderer();
		~BloomRenderer();
		bool Init(unsigned int windowWidth, unsigned int windowHeight);
		void Destroy();
		//void RenderBloomTexture(unsigned int srcTexture, float filterRadius);
		unsigned int BloomTexture();
		unsigned int BloomMip_i(int index);

		//void RenderDownsamples(unsigned int srcTexture);
		//void RenderUpsamples(float filterRadius);

		bool mInit;
		bloomFBO mFBO;
		glm::ivec2 mSrcViewportSize;
		glm::vec2 mSrcViewportSizeFloat;
		Pipeline* mDownsampleShader;
		Pipeline* mUpsampleShader;

		bool mKarisAverageOnDownsample = true;
	};

	BloomRenderer::BloomRenderer() : mInit(false) {}
	BloomRenderer::~BloomRenderer() {}

	bool BloomRenderer::Init(unsigned int windowWidth, unsigned int windowHeight)
	{
		if (mInit) return true;
		mSrcViewportSize = glm::ivec2(windowWidth, windowHeight);
		mSrcViewportSizeFloat = glm::vec2((float)windowWidth, (float)windowHeight);

		// Framebuffer
		const unsigned int num_bloom_mips = 6; // TODO: Play around with this value
		bool status = mFBO.Init(windowWidth, windowHeight, num_bloom_mips);
		if (!status)
		{
			std::cerr << "Failed to initialize bloom FBO - cannot create bloom renderer!\n";
			return false;
		}

		// Shaders
		mDownsampleShader = new Pipeline("data/shaders/ALL_with_Bloom/downsample.vert", "data/shaders/ALL_with_Bloom/downsample.frag");
		mUpsampleShader = new Pipeline("data/shaders/ALL_with_Bloom/downsample.vert", "data/shaders/ALL_with_Bloom/upsample.frag");

		// Downsample
		mDownsampleShader->use();
		mDownsampleShader->setInt("srcTexture", 0);
		glUseProgram(0);

		// Upsample
		mUpsampleShader->use();
		mUpsampleShader->setInt("srcTexture", 0);
		glUseProgram(0);

		return true;
	}

	void BloomRenderer::Destroy()
	{
		mFBO.Destroy();
		delete mDownsampleShader;
		delete mUpsampleShader;
	}

	GLuint BloomRenderer::BloomTexture()
	{
		return mFBO.MipChain()[0].texture;
	}

	GLuint BloomRenderer::BloomMip_i(int index)
	{
		const std::vector<bloomMip>& mipChain = mFBO.MipChain();
		int size = (int)mipChain.size();
		return mipChain[(index > size - 1) ? size - 1 : (index < 0) ? 0 : index].texture;
	}
}