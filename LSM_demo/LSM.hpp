#pragma once

#include "Run.hpp"
#include "buffer.hpp"
#include "DiskRun.hpp"
#include "Vector"
#include "BloomFilter.hpp"
#define FP_rate 0.1

template<typename K, typename V>
class LSM {
private:
    typedef Pair<K,V> KV_pair;
    Buffer<K,V>* buff;
    DiskRun<K,V>** runs;
    BloomFilter<K>** filters;
    int buffer_size;
    int max_level;
    int runs_per_level;
    int page_size;
    int current_level;
public:

	// sort two sorted arrays
	vector<KV_pair> trickySort(vector<KV_pair> new_vec, vector<KV_pair> old_vec) {
		int pointer1 = 0, pointer2 = 0;
		vector<KV_pair> merged;

		// two pointers scan thru the vectors
		while (pointer1 < new_vec.size() && pointer2 < old_vec.size()) {
			if (new_vec[pointer1] < old_vec[pointer2]) {
				merged.push_back(new_vec[pointer1]);
				pointer1 += 1;
			}
			else if (new_vec[pointer1] > old_vec[pointer2]) {
				merged.push_back(old_vec[pointer2]);
				pointer2 += 1;
			}
			else {	// equal -> check if it's TOMBSTONE
				if (new_vec[pointer1].value != TOMBSTONE)
					merged.push_back(new_vec[pointer1]);

				pointer1 += 1;
				pointer2 += 1;
			}
		}

		// append the leftover
		if (pointer1 < new_vec.size())
			merged.insert(merged.end(), new_vec.begin() + pointer1, new_vec.end());
		// append the leftover
		if (pointer2 < old_vec.size())
			merged.insert(merged.end(), old_vec.begin() + pointer2, old_vec.end());

		return merged;
	}

public:
    LSM(){

    }

    LSM(int buffer_size, int page_size, int max_level, int runs_per_level){
        int run_entries = buffer_size / sizeof(KV_pair);
        this->buffer_size = buffer_size;
        this->page_size = page_size;
        this->max_level = max_level;
        this->runs_per_level = runs_per_level;
<<<<<<< HEAD
        buff = new Buffer<K, V>(buffer_size/sizeof(KV_pair));
        runs = new DiskRun<K,V>[max_level];
=======
        current_level = 0;
        buff = new Buffer<K,V>(buffer_size/sizeof(KV_pair));
        runs = new DiskRun<K,V>*[max_level];
        for(int i = 0; i < max_level; i++){
            runs[i] = new DiskRun<K,V>(run_entries*(runs_per_level-1),page_size,i+1,1);
            run_entries *= runs_per_level;
        }
        run_entries = buffer_size / sizeof(KV_pair);
        filters = new BloomFilter<K>*[max_level];
        for(int i = 0; i < max_level; i++){
            filters[i] = new BloomFilter<K>(run_entries*runs_per_level,FP_rate);
            run_entries *= runs_per_level;
        }
>>>>>>> 92b3c46858575d50fa72d4b967f58b2d7ef61022
    }

	void insert(K key, V value) {
	    buff->insert(Pair<K,V>(key,value));
	    if(buff->isfull()){
	        runs[0]->merge(buff->push());
            buff->clear();
            filters[0]->addKey(key);
            current_level++;
            for(int i = 0; i < max_level; i++){
                if(i == max_level - 2){
                    if(runs[i+1]->exist() && runs[i+1]->overlimit(runs[i]->get_entries_num())){
                        runs[i+1]->removerun();
                        runs[i]->merge();
                    }
                }
                else if(runs[i]->overlimit()){
                    if(!runs[i+1]->exist()){
                        runs[i]->merge();
                        runs[i]->empty();
                    }
                    else{
                        runs[i+1]->merge(runs[i]->load());
                    }
                    filters[i+1]->addKey(key);
                    current_level++;
                }
                else{
                    break;
                }
            }
	    }
	}
<<<<<<< HEAD

	vector<Pair<K, V> > range(K key_min, K key_max) {
=======
>>>>>>> 92b3c46858575d50fa72d4b967f58b2d7ef61022

	vector<Pair<K, V> > range(K key_min, K key_max) {
	    vector<KV_pair> A,B;
	    A = buff->range(key_min, key_max);
	    for(int i = 0; i < current_level; i++){
            if(runs[i]->exist()){
                B = runs[i]->rangeSearch(key_min, key_max);
                A = trickySort(A,B);
            }
	    }
	    return A;
	}

	void delete_key(K key) {
	    Pair<K,V> aPair = {key,0};
	    buff->_delete(aPair);
	}
	Pair<K,V>* lookup(K key) {
	    Pair<K,V>* aPair;
	    Pair<K,V>* result= buff->lookup(key);
	    if(result == NULL){
            for(int i = 0; i < current_level; i++){
                if(filters[i]->contain(key)){
                    aPair = runs[i]->lookup(key);
                    if(aPair != NULL){
                        if(aPair->value == TOMBSTONE){
                            return NULL;
                        }
                        else return aPair;
                    }
                }
            }
	    }
	    else if(result->value == TOMBSTONE){
            return NULL;
	    }
	    else{
            return result;
	    }
	    return NULL;
	}
};
