#include <algorithm>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iterator>
#include <ctime>
#include <fstream>

#define DoSort false

bool print = true;

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
void swap(K *d, int i, int j) {
	K t = d[i];
	d[i] = d[j];
	d[j] = t;
}

// Given an array d, we copy the contents of d[src] 
// to d[dest].  Anything in det is lost.
template <typename K>
void move(K *d, int dest, int src) {
	d[dest] = d[src];
}

// Linearly search through the array s from start to end for
// the value v.  If found return its index, otherwise return
// -1
template <typename K>
int linearSearch(const K* s, K v, int start, int end) {
	while (start + 8 < end) {
		if (s[start + 0] == v) return start;
		if (s[start + 1] == v) return start;
		if (s[start + 2] == v) return start;
		if (s[start + 3] == v) return start;
		if (s[start + 4] == v) return start;
		if (s[start + 5] == v) return start;
		if (s[start + 6] == v) return start;
		if (s[start + 7] == v) return start;
		start += 8;
	}
	while (start < end) {
		if (s[start] == v) return start;
		++start;
	}
	return -1;
}

template <typename K>
int bSearch(const K* s, K v, int left, int right) {
	while (left < right) {
		int middle = (left + right) >> 1;
		asm("cmpl %3, %2\n\tcmovg %4, %0\n\tcmovle %5, %1"
			: "+r" (left),
			"+r" (right)
			: "r" (v), "g" (s[middle]),
			"g" (middle + 1), "g" (middle));
	}
	if (s[left] == v) return left;
	return -1;
}

// Given a sorted array we use binary search to search for
// the value v and return its position.  If not found we
// return -1
template <typename K>
int binarySearch(const K* s, K v, int left, int right) {

	while (left < right) {
		int mid = (left + right) / 2;
		if (s[mid] == v) {
			return mid;
		} else if (s[mid] < v) {	//  [left ... mid .V.. right]
			left = mid + 1;
		} else {						//  [left .V. mid .... right]
			right = mid;
		}
	}

	return -1;
}

// Try to convert a value to a string
template <typename T>
std::string to_string(T v) {
	std::ostringstream ss;
	ss << v;
	return ss.str();
}

template <typename K>
int search(K* s, K v, int start, int end) {
	if (DoSort) {
		return binarySearch(s, v, start, end);
	} else {
		return linearSearch(s, v, start, end);
	}
}

namespace DataStructures {

	template <typename T>
	class LinearHash {

		// Hard-coded defaults
		static const size_t NUM_BUCKETS = 100;
		static const size_t NUM_ELEMENTS = 32;

		// Forward declaration
		class Bucket;
		class MyIterator;

	public:
		LinearHash() {
			init();
		}

		LinearHash(size_t num_buckets) {
			num_buckets_ = num_buckets;
			init();
		}

		LinearHash(size_t num_buckets, size_t num_elements) {
			num_buckets_ = num_buckets;
			num_elements_ = num_elements;

			init();
		}

		~LinearHash() {
			for (size_t i = 0; i < num_buckets_; ++i) {
				if (buckets_[i] != NULL) {
					delete buckets_[i];
				}
			}
			delete[] buckets_;
		}

		// Given a key and value we place it in the hash table
		void put(size_t key, T *value) {

			// Get the bucket to place into
			int index = computeIndex(key);
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

		T *get(size_t key) {
			size_t index = computeIndex(key);
			Bucket *b = getBucket(index);
			if (b == NULL) {
				return NULL;
			}
			return b->get(key);
		}

		bool contains(size_t key) {
			return get(key) != NULL;
		}

		size_t bucket_count() {
			return num_buckets_;
		}

		size_t split_count() {
			return num_splits_;
		}
		
		size_t bucket_size() {
			return num_elements_;
		}
		size_t count() {
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

				for (size_t i = 0; i < num_buckets_; ++i) {
					r[i] = buckets_[i];
				}
				delete[] buckets_;
				buckets_ = r;
			}
		}

		void split() {
			const size_t second = num_buckets_ + num_splits_;
			Bucket *prev = getBucket(num_splits_);
			Bucket *next = getBucket(second);
			++num_splits_;

			while (prev != NULL) {

				int unmoved = 0;
				int end = prev->count_;

				for (int i = 0; i < end; ++i) {

					size_t idx = computeIndex(prev->keys_[i]);

					if (idx == second) {		// Move to other

						next->append(prev->keys_[i], prev->values_[i]);
						if (next->full()) {
							next->chain_ = new Bucket(num_elements_);
							next = next->chain_;
						}
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
				prev = prev->chain_;
			}

			prev = getBucket(num_splits_);
			next = getBucket(second);

			prev->compact();
			next->compact();


		}

		void init() {
			buckets_ = new Bucket*[num_buckets_];
			memset(buckets_, 0, sizeof(Bucket*)* num_buckets_);
		}

		size_t computeIndex(size_t key) {
			size_t m = key % num_buckets_;
			if (m < num_splits_) {
				m = key % (2 * num_buckets_);
			}
			return m;
		}

		Bucket *getBucket(size_t index) {
			if (buckets_[index] == NULL) {
				buckets_[index] = new Bucket(num_elements_);
			}
			return buckets_[index];
		}

		class MyTuple {
		public:
			size_t key;
			T* value;

			MyTuple(size_t k, T* v) {
				key = k;
				value = v;
			}

			size_t getKey() {
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
			size_t b_pos;
			size_t pos;
			size_t total;
			MyIterator end();

		public:

			MyIterator() {
				buckets = NULL;
				bucket = NULL;
				b_pos = pos = total = 0;
			}

			MyIterator(Bucket **bs, size_t num_buckets, size_t splits) {

				buckets = bs;
				bucket = bs[0];

				pos = b_pos = 0;

				while (bucket == NULL) {
					bucket = bs[++pos];
				}

				total = num_buckets + splits;

			}

			MyTuple operator*() {
				MyTuple ret(bucket->keys_[b_pos], bucket->values_[b_pos]);
				++b_pos;
				return ret;
			}

			MyIterator &operator++() {
				while (pos < total) {
					// Search current bucket for item
					while (bucket != NULL) {
						if (bucket->count_ == b_pos) {		// Next bucket
							b_pos = 0;
							bucket = bucket->chain_;
						} else {							// Found a spot!
							return *this;
						}
					}
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
			size_t total_elements_;
			size_t total_count_;

			// Local values
			size_t num_elements_;
			size_t count_;

			// Data
			size_t *keys_;
			T **values_;

			// Chain
			Bucket *chain_;

			Bucket(size_t num_elements) {

				total_elements_ = num_elements_ = num_elements;

				total_count_ = count_ = 0;

				// Data
				keys_ = new size_t[num_elements_];
				values_ = new T*[num_elements_];

				memset(values_, 0, sizeof(T*)* num_elements_);

				chain_ = NULL;

			}

			~Bucket() {

				count_ = 0;
				num_elements_ = 0;

				if (keys_) {
					delete[] keys_;
					delete[] values_;
				}

				if (chain_) {
					delete chain_;
				}

				chain_ = NULL;
				keys_ = NULL;
				values_ = NULL;

			}

			Bucket *getBucket(size_t key, size_t &index) {
				Bucket *curr = this;

				while (curr != NULL) {
					index = search(curr->keys_, key, 0, curr->count_);

					// Check to see if we stop here
					if (index != -1 || curr->chain_ == NULL) {
						return curr;
					}

					curr = curr->chain_;
				}
				return NULL;
			}


			// Special function only called when just created
			// Bucket and need to fill it right away
			void append(size_t key, T *value) {
				Bucket *curr = this;
				while (curr->full()) {
					if (chain_ == NULL) {
						chain_ = new Bucket(num_elements_);
					}
					curr = curr->chain_;
				}
				sort(key, value);
				++count_;
			}

			// Returns true if collision
			bool put(size_t key, T *value) {
				size_t index = -1;
				Bucket *bucket = this->getBucket(key, index);

				// If we found a previous key/value pair
				if (index != -1) {
					if (bucket->values_[index] != NULL) {
						delete bucket->values_[index];	// Remove old
					}
					bucket->values_[index] = value;	// Add new
					return true;
				}

				// Last bucket
				if (bucket->full()) {
					Assert("Chain is not null", bucket->chain_ == NULL);
					bucket->chain_ = new Bucket(num_elements_);
					bucket = bucket->chain_;
				}

				bucket->append(key, value);
				return true;
			}

			// Search for a value given a key in this bucket
			T *get(size_t key) {
				size_t index = -1;
				Bucket *ret = getBucket(key, index);
				if (index == -1) {
					return NULL;
				}
				return ret->values_[index];
			}

			T *remove(size_t key) {
				Assert("Remove was called", false);
				Bucket *curr = this;
				while (curr != NULL) {
					int pos = search(curr->_keys, key, 0, curr->count_);
					if (pos != -1) {
						T *ret = curr->values_[pos];
						--(curr->count_);
						while (pos < curr->count) {
							move(curr->keys_, pos, pos + 1);
							move(curr->values_, pos, pos + 1);
							++pos;
						}
						return ret;
					}
					curr = curr->chain_;
				}
				return NULL;
			}

			bool full() {
				return count_ == num_elements_;
			}

			bool contains(size_t key) {
				return get(key);
			}

			// Not implemented yet
			void compact() {
				Bucket *curr = this;
				while (curr->chain_ != NULL) {
					if (full()) {
						curr = curr->chain_;
						continue;
					}

					Bucket *t = curr->chain_;

					// If nothing in the next remove him
					if (curr->chain_->count_ == 0) {
						// Reconnect
						curr->chain_ = t->chain_;

						// Remove
						t->chain_ = NULL;
						delete t;
					} else {	// Copy what we can over
						--t->count_;
						size_t key = t->keys_[t->count_];
						T *value = t->values_[t->count_];

						curr->append(key, value);
					}
				}
			}

			// Just inserted at the end, sort
			void sort(size_t k, T* v) {
				int i = count_;
				if (DoSort) {
					// While our key is strictly smaller than the previous key
					while (i > 0 && keys_[i - 1] > k) {
						// Copy them up
						keys_[i] = keys_[i - 1];
						values_[i] = values_[i - 1];
						--i;
					}
				}
				keys_[i] = k;
				values_[i] = v;

			}

		};

		MyIterator end_;
		size_t num_buckets_ = NUM_BUCKETS;
		size_t num_elements_ = NUM_ELEMENTS;
		size_t count_ = 0;
		size_t num_splits_ = 0;
		size_t num_items_ = 0;
		Bucket **buckets_ = NULL;

	};

}

#define max(A,B) (A) > (B) ? (A) : (B)

void dumpToFile( std::string filename , DataStructures::LinearHash<std::string> &hash ) {
	std::ofstream outfile( filename , std::ofstream::binary );
	// header
	
	size_t s_buff;
	const char* buffer = (const char*)&s_buff;
	
	s_buff = hash.bucket_count();
	outfile.write( buffer	, sizeof(size_t) );	// How many buckets
	
	s_buff = hash.split_count();
	outfile.write( buffer	, sizeof(size_t) );	// How many splits
	
	s_buff = hash.bucket_size();
	outfile.write( buffer	, sizeof(size_t) );	// How many elements
	
	s_buff = hash.count();
	outfile.write( buffer	, sizeof(size_t) );	// How many keys inserted
	
	// Data
	for( auto iter = hash.begin() ; iter != hash.end() ; ++iter ) {
		auto pair = *iter;
		std::string *value	= pair.getValue();
		
		size_t s_buff;
		const char* buffer = (const char*)&s_buff;
		
		// Print key
		s_buff = pair.getKey();
		outfile.write( buffer , sizeof(size_t) );
		
		// Print length
		s_buff = value->length();
		outfile.write( buffer , sizeof(size_t) );
		
		// Print string
		outfile.write( value->c_str() , value->length() );
		
	}
	outfile.close();
}

void readFromFile( std::string filename , DataStructures::LinearHash<std::string> &hash ) {
	std::ifstream infile( filename , std::ofstream::binary );
	
	size_t s_buff;
	const char* buffer = (const char*)&s_buff;
	
	size_t num_buckets,splits,num_elements,count;
	
	s_buff = hash.bucket_count();
	infile.read( (char*)&num_buckets	, sizeof(size_t) );	// How many buckets
	
	s_buff = hash.split_count();
	infile.read( (char*)&splits		, sizeof(size_t) );	// How many splits
	
	s_buff = hash.bucket_size();
	infile.read( (char*)&num_elements	, sizeof(size_t) );	// How many elements
	
	s_buff = hash.count();
	infile.read( (char*)&count		, sizeof(size_t) );	// How many keys inserted
	
	// I would hope this is large enough...
	char *str_buffer = new char[1024*1024];
	
	// Data
	for( int i = 0 ; i < count ; ++i ) {
		size_t key,length;
		
		infile.read( (char*)&key , sizeof(size_t) );
		infile.read( (char*)&length , sizeof(size_t) );
		infile.read( str_buffer , length );
		
		hash.put( key , new std::string(str_buffer, length) );
	}
	
	delete[] str_buffer;
	infile.close();
}

int main(void) {
	DataStructures::LinearHash<std::string> fromFile( 1024 , 16 );
	DataStructures::LinearHash<std::string> myHash(1024, 16);
	
	int count = 0;
	
	std::string insert("ABCDEFGHI");
	std::string fetch("ABCDEFGHI");
	std::string file("ABCDEFGHI");
	std::hash<std::string> str_hash;

	clock_t start,end;
	start = std::clock();
	do {	// 362880 permutations
		myHash.put(str_hash(insert), new std::string(insert));
	} while (std::next_permutation(insert.begin(), insert.end()));
	end = std::clock();
	std::cout << "Took " << 1000 * (float)(end - start) / CLOCKS_PER_SEC << "ms to insert." << std::endl;

	start = std::clock();
	do {
		++count;
		if (!myHash.contains(str_hash(fetch))) {
			std::cout << "ERROR: " << fetch << " at " << count << std::endl;
			return -1;
		}
	} while (std::next_permutation(fetch.begin(), fetch.end()));
	end = std::clock();

	std::cout << "Took " << 1000 * (float)(end - start) / CLOCKS_PER_SEC << "ms to fetch." << std::endl;
	std::cout << "Hashmap contains " << myHash.count() << " items" << std::endl;
	std::cout << "Total buckets: " << myHash.bucket_count() << std::endl;

	dumpToFile( "output.dat" , myHash );
	readFromFile( "output.dat" , fromFile );
	
	do {
		++count;
		if (!fromFile.contains(str_hash(file))) {
			std::cout << "ERROR: " << file << " at " << count << std::endl;
			return -1;
		}
	} while (std::next_permutation(file.begin(), file.end()));
	
	return 0;
}