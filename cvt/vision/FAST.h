#ifndef CVT_FAST_H
#define CVT_FAST_H

#include <cvt/vision/FeatureExtractor.h>

namespace cvt
{
	enum FASTSize {
		SEGMENT_9,
		SEGMENT_10,
		SEGMENT_11,
		SEGMENT_12
	};

	class FAST : public FeatureExtractor<int32_t>
	{
		typedef void (FAST::*ExtractFunc)( const uint8_t* im, size_t stride, size_t width, size_t height, std::vector<Feature2D> & features );
		typedef int (FAST::*ScoreFunc)( const uint8_t* p, const int pixel[] );

		public:
			FAST( FASTSize size = SEGMENT_9 );
			~FAST();

			void extract( const Image & image, std::vector<Feature2D> & features );
			void extractMultiScale( const Image & image, std::vector<Feature2D> & features, size_t octaves );
			void setThreshold( uint8_t threshold );
			uint8_t threshold();
			void setMinScore( int32_t minscore );
			int32_t minScore();

		private:
			uint8_t		_threshold;
			int32_t		_minScore;
			ExtractFunc	_extract;
			ScoreFunc	_score;

			int* score( const uint8_t* img, size_t stride, std::vector<Feature2D> & corners );

			void detect9( const uint8_t* im, size_t stride, size_t width, size_t height, std::vector<Feature2D> & features );
			int score9( const uint8_t* p, const int pixel[] );
			void detect10( const uint8_t* im, size_t stride, size_t width, size_t height, std::vector<Feature2D> & features );
			int score10( const uint8_t* p, const int pixel[] );
			void detect11( const uint8_t* im, size_t stride, size_t width, size_t height, std::vector<Feature2D> & features );
			int score11( const uint8_t* p, const int pixel[] );
			void detect12( const uint8_t* im, size_t stride, size_t width, size_t height, std::vector<Feature2D> & features );
			int score12( const uint8_t* p, const int pixel[] );

			void make_offsets(int pixel[], size_t row_stride);
			void nonmaxSuppression( const std::vector<Feature2D> & corners, const int* scores, std::vector<Feature2D> & suppressed );
	};

	inline void FAST::setThreshold( uint8_t threshold )
	{
		_threshold = threshold;
	}

	inline uint8_t FAST::threshold()
	{
		return _threshold;
	}

	inline void FAST::setMinScore( int32_t minscore )
	{
		_minScore = minscore;
	}

	inline int32_t FAST::minScore()
	{
		return _minScore;
	}
}

#endif
