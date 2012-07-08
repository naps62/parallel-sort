#define _TBB

#include <vector>
using namespace std;

#include <tbb/tbb.h>
using namespace tbb;

typedef unsigned int uint;
typedef concurrent_vector<concurrent_vector<uint> > bucket_list_t;
typedef concurrent_vector<uint> radix_arr_t;

#include "common.shared_mem.cpp"

	//
	// container class for all radix data
	//
	struct Container {
			bucket_list_t buckets;
			concurrent_vector<uint> bucket_accum;
			radix_arr_t *arr;

			uint mask;
			uint b;
			uint g;
			uint round;

			Container(radix_arr_t *arr, uint g, uint b) 
			: buckets(b),
			  bucket_accum(b)
			{
				this->arr = arr;
				this->mask = mask;
				this->g = g;
				this->b    = b;
				this->mask = (b - 1);

				bucket_accum[0] = 0;
			}

	};

	//
	// bucket clear functor
	//
	class BucketClear {
		private:
		Container *c;

		public:
		BucketClear(Container *container) : c(container) { }

		void operator()(const blocked_range<uint>& r) const {
			for(uint i=r.begin(); i!=r.end(); ++i) {
				c->buckets[i].clear();
			}
		}
	};

	//
	// bucket fill functor
	//
	class BucketFill {
		private:
		Container *c;

		public:
		BucketFill(Container *container) : c(container) { }

		void operator()(const blocked_range<uint>& r) const {
			for(uint i = 0; i < c->arr->size(); ++i) {
				uint elem = (*(c->arr))[i];
				uint bucket = GET_BUCKET_NUM(elem, c->mask, c->g, c->round);

				for(uint b=r.begin(); b!=r.end(); ++b)
					if (b == bucket) {
						c->buckets[bucket].push_back(elem);	
						break;
					}
			}
		}
	};

	//
	// bucket gather onto final array (finishing one iteration)
	//
	class BucketGather {
		private:
		Container *c;

		public:
		BucketGather(Container *container) : c(container) { }

		void operator()(const blocked_range<uint>& r) const {
			for(uint i=r.begin(); i!=r.end(); ++i) {
				uint base_index = c->bucket_accum[i];

				for(uint elem=0; elem < c->buckets[i].size(); ++elem) {
					(*(c->arr))[base_index + elem] = c->buckets[i][elem];
				}
			}
		}
	};

void radix(radix_arr_t &arr, const unsigned int num_threads, const unsigned int g) {
	tbb::task_scheduler_init init(num_threads);

	Container    c(&arr, g, 1<<g);
	BucketClear  task_clear(&c);
	BucketFill   task_fill(&c);
	BucketGather task_gather(&c);

	//
	// for each digit
	//
	for(c.round = 0; c.mask != 0; c.mask <<= g, ++c.round) {
		
		//
		// clear buckets
		//
		parallel_for(blocked_range<uint>(0, c.b), task_clear);

		//
		// fill buckets
		//
		parallel_for(blocked_range<uint>(0, c.b), task_fill);

		//
		// calc bucket_accum
		//
		unsigned int sum = c.buckets[0].size();
		for(unsigned int i = 1; i < c.b; ++i) {
			c.bucket_accum[i] = sum;
			sum += c.buckets[i].size();
		}

		for(uint i = 0; i < c.b; ++i)
			cout << c.buckets[i].size() << ", ";
		cout << endl;

		//
		// and gather everything to main array
		//
		parallel_for(blocked_range<uint>(0, c.b), task_gather);


	for(uint i = 0; i < arr.size(); ++i)
		cout << arr[i] << ", ";
	cout << endl;
	}	
}
