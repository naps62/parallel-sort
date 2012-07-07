#define _TBB
#include "common.shared_mem.cpp"
#include <tbb/tbb.h>
using namespace tbb;

typedef unsigned int uint;
typedef concurrent_vector<concurrent_vector<uint> > bucket_list_t;
typedef concurrent_vector<uint> radix_arr_t;

namespace RadixTasks {

	//
	// bucket clear functor
	//
	class BucketClear {
		private:
		const bucket_list_t *buckets;

		public:
		BucketClear(bucket_list_t *buckets) : buckets(buckets) { }

		void operator()(const blocked_range<uint>& r) {
			for(uint i=r.begin(); i!=r.end(); ++i) {
				(*buckets)[i]clear();
			}
		}
	};

	//
	// bucket fill functor
	//
	class BucketFill {
		private:
		const bucket_list_t *buckets;
		const radix_arr_t *arr;

		public:
		BucketFill(bucket_list_t *buckets, radix_arr_t *arr)
		: buckets(buckets), arr(arr) { }

		void operator()(const blocked_range<uint>& r) {
			for(uint i = r.begin(); i < r.end(); ++i) {
				uint elem = (*arr)[i];
				uint bucket = GET_BUCKET_NUM(elem, mask, g, round);

				(*buckets)[bucket].push_back(elem);
			}
		}
	};

	//
	// bucket gather onto final array (finishing one iteration)
	//
	class BucketGather {
		private:
		const bucket_list_t *buckets;
		const concurrent_vector<uint> *bucket_accum;
		const radix_arr_t *arr;		

		public:
		BucketGather(bucket_list_t *buckets, concurrent_vector<uint> *bucket_accum, radix_arr_t *arr)
		: buckets(buckets), bucket_accum(bucket_accum), arr(arr) { }

		void operator()(const blocked_range<uint>& r) {
			for(uint i=r.begin(); i!=r.end(); ++i) {
				uint base_index = bucket_accum[i];

				for(uint elem=0; elem<buckets[bucket].size(); ++elem) {
					arr[base_index + elem] = buckets[bucket][elem];
				}
			}
		}
	};
}

void radix_omp(radix_arr_t, const unsigned int num_threads, const unsigned int g) {
	tbb::task_scheduler_init init(num_threads);

	const unsigned int b	= (1 << g);				      // num of buckets (2^g)
	unsigned int bpp		= b / num_threads;	   	// num of buckets per thread
	const unsigned int bpp_bc = popcount(bpp - 1);	// number of bits in bpp. used to compute BUCKET_TO_CPU

	typedef concurrent_vector<concurrent_vector<unsigned int> > bucket_list_t
	bucket_list_t buckets(b);	// the buckets
	concurrent_vector<unsigned int> bucket_accum(b);		// accumulated values for bucket_count. indicates where the values for each bucket should go
	bucket_accum[0] = 0;

	unsigned int thread_num = omp_get_thread_num();
	unsigned int mask = ((1 << g) - 1);		// initial mask to get key

	filter_t<vector<unsigned int>, 

	//
	// for each digit
	//
	for(unsigned int round = 0; mask != 0; mask <<= g, ++round) {
		
		//
		// clear buckets
		//
		parallel_for(blocked_range<uint>(0, b), BucketClear(&buckets));

		//
		// fill buckets
		//
		parallel_for(blocked_range<uint>(0, arr.size()), BucketFill(&buckets, &arr));

		//
		// calc bucket_accum
		//
		unsigned int sum = buckets[0].size();
		for(unsigned int i = 1; i < b; ++i) {
			bucket_accum[i] = sum;
			sum += buckets[i].size();
		}

		//
		// and gather everything to main array
		//
		parallel_for(blocked_range<uint>(0,b)), BucketGather(&buckets, &bucket_accum, &arr));
	}	
}