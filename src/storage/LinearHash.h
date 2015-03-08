#ifndef _LINEARHASH_H_
#define _LINEARHASH_H_

#include <algorithm>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iterator>
#include <ctime>
#include <fstream>
#include <limits>

#ifdef _MSC_VER

typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;

#else
#include <stdint.h>
#endif

// Globals
const uint64_t INF = std::numeric_limits<uint64_t>::max();
const bool print = true;


// Defines
#define tests true
#define DoSort true
#define max(A,B) (A) > (B) ? (A) : (B)


// Helper Functions

// If the test fails we print out the message and then exit
void Assert(std::string msg, bool test) {
	if (!test) {
		std::cerr << msg << std::endl;
		exit(-1);
	}
}

// Given an array d, we swap elements i and j
template <typename K>
void swap(K *d, uint64_t i, uint64_t j) {
	K t = d[i];
	d[i] = d[j];
	d[j] = t;
}

// Given an array d, we copy the contents of d[src] 
// to d[dest].  Anything in det is lost.
template <typename K>
void move(K *d, uint64_t dest, uint64_t src) {
	d[dest] = d[src];
}

// Try to convert a value to a string
template <typename T>
std::string to_string(T v) {
	std::ostringstream ss;
	ss << v;
	return ss.str();
}


// Searching

// Linearly search through the array s from start to end for
// the value v.  If found return its index, otherwise return
// INF
template <typename K>
uint64_t linearSearch(const K* s, K v, register uint64_t start, const uint64_t end) {
	const uint64_t jump = 16;
	register uint64_t test = start + jump - 1;
	while (test < end ) {

		//*
		if (s[start + 0] == v) return start + 0;
		if (s[start + 1] == v) return start + 1;
		if (s[start + 2] == v) return start + 2;
		if (s[start + 3] == v) return start + 3;
		// /*
		if (s[start + 4] == v) return start + 4;
		if (s[start + 5] == v) return start + 5;
		if (s[start + 6] == v) return start + 6;
		if (s[start + 7] == v) return start + 7;
		// */
		
		///*
		if (s[start + 8] == v) return start + 8;
		if (s[start + 9] == v) return start + 9;
		if (s[start + 10] == v) return start + 10;
		if (s[start + 11] == v) return start + 11;

		if (s[start + 12] == v) return start + 12;
		if (s[start + 13] == v) return start + 13;
		if (s[start + 14] == v) return start + 14;
		if (s[start + 15] == v) return start + 15;
		// */
			
		start += jump;
		test += jump;
	}
	while (start < end) {
		if (s[start] == v) {
			return start;
		}
		++start;
	}
	return INF;
}

// Given a sorted array we use binary search to search for
// the value v and return its position.  If not found we
// return INF
template <typename K>
uint64_t binarySearch(const K* s, K v, uint64_t left, uint64_t right) {

	while (left < right) {
		uint64_t mid = (left + right) / 2;
		if (s[mid] == v) {
			return mid;
		} else if (s[mid] < v) {	//  [left ... mid .V.. right]
			left = mid + 1;
		} else {						//  [left .V. mid .... right]
			right = mid;
		}
	}

	return INF;
}

template <typename K>
uint64_t search(K* s, K v, uint64_t start, uint64_t end) {
	if (DoSort) {
		return binarySearch(s, v, start, end);
	} else {
		return linearSearch(s, v, start, end);
	}
}


// Actual hash table

namespace DataStructures {

	template <typename T>
	class LinearHash {

		// Hard-coded defaults
		static const uint64_t NUM_BUCKETS = 100;
		static const uint64_t NUM_ELEMENTS = 32;

		// Forward declaration
		class Bucket;
		class MyIterator;

	public:
		LinearHash() {
			init();
		}

		LinearHash(uint64_t num_buckets) {
			num_buckets_ = num_buckets;
			init();
		}

		LinearHash(uint64_t num_buckets, uint64_t num_elements) {
			num_buckets_ = num_buckets;
			num_elements_ = num_elements;

			init();
		}

		LinearHash(LinearHash &other) {
			num_buckets_ = other.num_buckets_;
			num_elements_ = other.num_elements_;

			init();

			count_ = other.count_;
			num_splits_ = other.num_splits_;
			num_items_ = other.num_items_;

			uint64_t total = num_buckets_ + num_splits_;

			for (uint64_t i = 0; i < total; ++i) {
				if (other.buckets_[i] != NULL) {
					buckets_[i] = new Bucket(other.buckets[i]);
				}
			}
		}

		~LinearHash() {
			for (uint64_t i = 0; i < num_buckets_ + num_splits_; ++i) {
				if (buckets_[i] != NULL) {
					delete buckets_[i];
				}
			}
			delete[] buckets_;
		}

		// Given a key and value we place it in the hash table
		void put(uint64_t key, T *value) {

			// Get the bucket to place into
			uint64_t index = computeIndex(key);
			Bucket *b = getBucket(index);

			// If full we need to "split"
			if (b->full()) {

				expand();		// Grow the hash table
				split();		// split the bucket at S

				if (num_splits_ == num_buckets_) {	// reset if we doubled
					num_splits_ = 0;
					num_buckets_ *= 2;
				}

				// Might be in a different bucket so fetch again
				index = computeIndex(key);
				b = getBucket(index);

			}

			b->put(key, value);
			++num_items_;	// How many items in the table
		}

		T *get(uint64_t key) {
			uint64_t index = computeIndex(key);
			Bucket *b = getBucket(index);
			if (b == NULL) {
				return NULL;
			}
			return b->get(key);
		}

		T *remove(uint64_t key) {
			uint64_t index = computeIndex(key);
			Bucket *b = getBucket(index);
			if (b == NULL) {
				return NULL;
			}
			return b->remove(key);
		}

		bool contains(uint64_t key) {
			return get(key) != NULL;
		}

		uint64_t bucket_count() {
			return num_buckets_;
		}

		uint64_t split_count() {
			return num_splits_;
		}

		uint64_t bucket_size() {
			return num_elements_;
		}

		uint64_t count() {
			return num_items_;
		}

		MyIterator begin() {
			return MyIterator(buckets_, num_buckets_, num_splits_);
		}

		const MyIterator &end() {
			return end_;
		}

	private:

		void expand() {
			// If actually full
			if (num_splits_ == 0) {
				Bucket **r = new Bucket*[2 * num_buckets_];				// allocate new bucket
				Assert("Error: could not grow Linear Hash Table", r != 0);

				memset(r, 0, sizeof(Bucket*)* 2 * num_buckets_);

				for (uint64_t i = 0; i < num_buckets_; ++i) {
					r[i] = buckets_[i];
				}
				delete[] buckets_;
				buckets_ = r;
			}
		}

		void split() {
			const uint64_t second = num_buckets_ + num_splits_;
			Bucket *prev = getBucket(num_splits_);
			Bucket *next = getBucket(second);
			++num_splits_;

			uint64_t unmoved = 0;
			uint64_t end = prev->count_;

			for (uint64_t i = 0; i < end; ++i) {

				uint64_t idx = computeIndex(prev->keys_[i]);

				if (idx == second) {		// Move to other

					next->append(prev->keys_[i], prev->values_[i]);
					prev->values_[i] = NULL;
				} else {
					if (unmoved != i) {	// If not in their place already, then move
						prev->keys_[unmoved] = prev->keys_[i];
						prev->values_[unmoved] = prev->values_[i];
					}
					++unmoved;	// Didn't move so count
				}
			}

			prev->count_ = unmoved;
			
		}

		void init() {
			buckets_ = new Bucket*[num_buckets_];
			memset(buckets_, 0, sizeof(Bucket*)* num_buckets_);
		}

		uint64_t computeIndex(uint64_t key) {
			uint64_t m = key % num_buckets_;
			if (m < num_splits_) {
				m = key % (2 * num_buckets_);
			}
			return m;
		}

		Bucket *getBucket(uint64_t index) {
			if (buckets_[index] == NULL) {
				buckets_[index] = new Bucket(num_elements_);
			}
			return buckets_[index];
		}

		class MyTuple {
		public:
			uint64_t key;
			T* value;

			MyTuple(uint64_t k, T* v) {
				key = k;
				value = v;
			}

			uint64_t getKey() {
				return key;
			}

			T* getValue() {
				return value;
			}
		};

		class MyIterator {
		private:
			Bucket **buckets;
			Bucket *bucket;
			uint64_t b_pos;
			uint64_t pos;
			uint64_t total;
			MyIterator end();

		public:

			MyIterator() {
				buckets = NULL;
				bucket = NULL;
				b_pos = pos = total = 0;
			}

			MyIterator(Bucket **bs, uint64_t num_buckets, uint64_t splits) {

				buckets = bs;
				bucket = bs[0];

				pos = b_pos = 0;

				total = num_buckets + splits;
				while (pos < total) {
					if (bucket == NULL) {
						bucket = bs[++pos];
					} else {
						break;
					}
				}

				if (pos == total) {
					std::cout << "uh..." << std::endl;
					buckets = NULL;
					bucket = NULL;
					b_pos = pos = total = 0;
				}


			}

			MyTuple operator*() {
				MyTuple ret(bucket->keys_[b_pos], bucket->values_[b_pos]);
				++b_pos;
				return ret;
			}

			MyIterator &operator++() {
				while (pos < total) {
					// Search current bucket for item
					if (bucket->count_ != b_pos) {		// Next bucket
						return *this;
					}
					b_pos = 0;
					++pos;
					bucket = buckets[pos];
				}

				buckets = NULL;
				bucket = NULL;
				pos = total = b_pos = 0;
				return *this;
			}

			MyIterator operator++(int) {
				return this++;
			}

			bool operator==(const MyIterator &other) const {
				return buckets == other.buckets && pos == other.pos && b_pos == other.b_pos;
			}

			bool operator!=(const MyIterator &other) const {
				return !(*this == other);
			}
		};

		class Bucket {

		public:
			// Global values
			uint64_t total_elements_;
			uint64_t total_count_;

			// Local values
			uint64_t num_elements_;
			uint64_t count_;

			// Data
			uint64_t *keys_;
			T **values_;


			Bucket(uint64_t num_elements) {

				total_elements_ = num_elements_ = num_elements;
				total_count_ = count_ = 0;

				// Data
				keys_ = new uint64_t[num_elements_];
				values_ = new T*[num_elements_];

				memset(values_, 0, sizeof(T*)* num_elements_);

			}

			Bucket(Bucket &other) {
				total_elements_ = num_elements_ = other.num_elements_;
				total_count_ = count_ = other.count_;

				// Data
				keys_ = new uint64_t[num_elements_];
				values_ = new T*[num_elements_];

				for (uint64_t i = 0; i < count_; ++i) {
					keys_[i] = other.keys_[i];
					values_[i] = other.values_[i];
				}

			}

			~Bucket() {

				count_ = 0;
				num_elements_ = 0;

				if (keys_ != NULL) {
					delete[] keys_;
					delete[] values_;
				}

				keys_ = NULL;
				values_ = NULL;

			}
			
			void resize() {
				uint64_t new_size = 2* num_elements_;
				
				uint64_t *new_keys = new uint64_t[new_size];
				T **new_values	= new T*[new_size];
				
				Assert( "Bucket could not grow" , new_keys != NULL );
				Assert( "Bucket could not grow" , new_values != NULL );
				
				for( int i = 0 ; i < num_elements_ ; ++i ) {
					new_keys[i] = keys_[i];
					new_values[i] = values_[i];
				}
				
				delete[] keys_;
				delete[] values_;
				
				num_elements_ = new_size;
				keys_ = new_keys;
				values_ = new_values;
			}


			// Special function only called when just created
			// Bucket and need to fill it right away
			void append(uint64_t key, T *value) {
				Bucket *curr = this;
				while (curr->full()) {
					resize();					
				}
				curr->sort(key, value);
				++curr->count_;
			}

			// Returns true if collision
			bool put(uint64_t key, T *value) {
				
				uint64_t index = search( keys_ , key , 0 , count_ );
				if( index != INF ) {
					if (values_[index] != NULL) {
						delete values_[index];	// Remove old
					}
					values_[index] = value;	// Add new
					return true;
				}
				append( key , value );
                                return false;
			}

			// Search for a value given a key in this bucket
			T *get(uint64_t key) {
				uint64_t index = search( keys_ , key , 0 , count_ );
				if (index == INF) {
					return NULL;
				}
				return values_[index];
			}

			T *remove(uint64_t key) {
				// Find my bucket
				uint64_t index = search( keys_ , key , 0 , count_ );
				
				// We don't actually have it
				if (index == INF) {
					return NULL;
				}

				// Remove from bucket, keep sorted maybe
				T* v = values_[index];
				--count_;
				
				if (DoSort) {
					while (index < count_) {
						swap(keys_, index, index + 1);
						swap(values_, index, index + 1);
						++index;
					}
				} else {
					swap(keys_, index, count_);
					swap(values_, index, count_);
				}
				
				return v;
			}

			bool full() {
				return count_ == num_elements_;
			}

			// Binary search
			void sort(uint64_t k, T* v) {
				uint64_t i = count_;
				if (DoSort) {
					//*
					// While our key is strictly smaller than the previous key
					while (i > 0 && keys_[i - 1] > k) {
						// Copy them up
						keys_[i] = keys_[i - 1];
						values_[i] = values_[i - 1];
						--i;
					}
					// */
				}
				keys_[i] = k;
				values_[i] = v;
			}

			void print(uint64_t pos) {
				if (pos > count_) return;

				print(pos * 2 + 1);

				auto i = pos;
				while (i > 0) { std::cout << "  "; i /= 2; }
				std::cout << " " << keys_[pos] << " " << std::endl;;

				print(pos * 2 + 2);
			}


		};

		MyIterator end_;
		uint64_t num_buckets_ = NUM_BUCKETS;
		uint64_t num_elements_ = NUM_ELEMENTS;
		uint64_t count_ = 0;
		uint64_t num_splits_ = 0;
		uint64_t num_items_ = 0;
		Bucket **buckets_ = NULL;

	};

}




//
// Format
//
//	Number of Buckets | Number of Elements per bucket | Number of total elements | Elements
//
//	Each element is composed of three parts:
//
//		Key : 64-bits
//		Length:	64-bits long
//		Data: Length bytes
//
//

// File I/O
void dumpToFile(std::string filename, DataStructures::LinearHash<std::string> &hash) {
	std::ofstream outfile(filename, std::ofstream::binary);
	// header
	uint64_t num_buckets = hash.bucket_count();
	if (hash.split_count() > 0) num_buckets += num_buckets;
	outfile.write(reinterpret_cast<char*>(&num_buckets), sizeof(uint64_t));	// How many buckets

	uint64_t size_bucket = hash.bucket_size();
	outfile.write(reinterpret_cast<char*>(&size_bucket), sizeof(uint64_t));	// How many buckets

	uint64_t num_elements = hash.count();
	outfile.write(reinterpret_cast<char*>(&num_elements), sizeof(uint64_t));	// How many buckets

	std::cout << "Writing out "
		<< num_buckets << " buckets, with "
		<< size_bucket << " elements per bucket, for a total of "
		<< num_elements << std::endl;



	int count = 0;
	// Data
	for (auto iter = hash.begin(); iter != hash.end(); ++iter) {
		++count;
		auto pair = *iter;

		uint64_t key = pair.getKey();
		uint64_t  length = pair.getValue()->length();
		const char* data = pair.getValue()->c_str();

		// Key
		outfile.write(reinterpret_cast<char*>(&key), sizeof(key));

		// Length
		outfile.write(reinterpret_cast<char*>(&length), sizeof(length));

		// Data
		outfile.write(data, length);

	}

	Assert("Missing some items", count == num_elements);
	outfile.close();
}


DataStructures::LinearHash<std::string> *readFromFile(std::string filename) {
	std::ifstream infile(filename, std::ofstream::binary);
	DataStructures::LinearHash<std::string> *result;

	uint64_t num_buckets, size_bucket, num_elements, count;

	infile.read(reinterpret_cast<char*>(&num_buckets), sizeof(uint64_t));	// How many buckets

	infile.read(reinterpret_cast<char*>(&size_bucket), sizeof(uint64_t));	// How many elements per bucket

	infile.read(reinterpret_cast<char*>(&num_elements), sizeof(uint64_t));		// How many elements

	std::cout << "Reading in "
		<< num_buckets << " buckets, with "
		<< size_bucket << " elements per bucket, for a total of "
		<< num_elements << std::endl;

	// I would hope this is large enough...
	char *str_buffer = new char[1024 * 1024];

	result = new DataStructures::LinearHash<std::string>(num_buckets, size_bucket);

	// Data
	for (uint64_t i = 0; i < num_elements; ++i) {
		uint64_t key, length;

		infile.read(reinterpret_cast<char*>(&key), sizeof(uint64_t));
		infile.read(reinterpret_cast<char*>(&length), sizeof(uint64_t));
		infile.read(str_buffer, length);

		result->put(key, new std::string(str_buffer, length));
	}

	infile.close();
	delete[] str_buffer;

	return result;
}



// Hash function
uint64_t hash3(const void * key, int len, unsigned int seed = 0) {
	const unsigned int m = 0x5bd1e995;
	const int r = 24;

	unsigned int h1 = seed ^ len;
	unsigned int h2 = 0;

	const unsigned int * data = (const unsigned int *)key;

	while (len >= 8) {
		unsigned int k1 = *data++;
		k1 *= m; k1 ^= k1 >> r; k1 *= m;
		h1 *= m; h1 ^= k1;
		len -= 4;

		unsigned int k2 = *data++;
		k2 *= m; k2 ^= k2 >> r; k2 *= m;
		h2 *= m; h2 ^= k2;
		len -= 4;
	}

	if (len >= 4) {
		unsigned int k1 = *data++;
		k1 *= m; k1 ^= k1 >> r; k1 *= m;
		h1 *= m; h1 ^= k1;
		len -= 4;
	}

	switch (len) {
	case 3: h2 ^= ((unsigned char*)data)[2] << 16;
	case 2: h2 ^= ((unsigned char*)data)[1] << 8;
	case 1: h2 ^= ((unsigned char*)data)[0];
		h2 *= m;
	};

	h1 ^= h2 >> 18; h1 *= m;
	h2 ^= h1 >> 22; h2 *= m;
	h1 ^= h2 >> 17; h1 *= m;
	h2 ^= h1 >> 19; h2 *= m;

	uint64_t h = h1;

	h = (h << 32) | h2;

	return h;
}

uint64_t hash2(const char* data, size_t len) {
	uint64_t hash = 0;
	const uint64_t mul = 101;
	for (uint64_t i = 0; i < len; ++i) {
		uint64_t c = data[i];
		hash = mul * hash + c;
	}
	return hash;
}

uint64_t hash1(const char* data, size_t length) {
	uint64_t result = static_cast<uint64_t>(14695981039346656037ULL);
	for (; length; --length) {
		result ^= static_cast<uint64_t>(*data++);
		result *= static_cast<uint64_t>(1099511628211ULL);
	}
	return result;
}

template<typename T>
inline uint64_t hash(T &str, size_t len) {
	//return hash1( str.c_str() , len );
	//return hash2( str.c_str() , len );
	return hash3(str.c_str(), len);		// Murmer
}

#endif