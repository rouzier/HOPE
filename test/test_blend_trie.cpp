#include "gtest/gtest.h"

#include <assert.h>

#include <bitset>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

#include "blending_trie.hpp"

namespace ope {

	namespace blendtrietest {

		static const std::string kEmailFilePath = "../../datasets/emails.txt";
		static const int kEmailTestSize = 100000;
		static std::vector<std::string> emails;

		static const std::string kBlendingResultPath = "../../datasets/blend_results_" +
														std::to_string(kEmailTestSize) + ".txt";
		static std::vector<SymbolFreq> blend_results;
		static std::vector<SymbolFreq> blend_freq_vec;
		static std::map<std::string, int64_t> blend_freq_map;

		class BlendTrieTest : public ::testing::Test {
		public:
			void getFrequencyTable(const std::vector<std::string>& key_list,
								std::map<std::string, int64_t>& freq_map_);
		};

		void BlendTrieTest::getFrequencyTable(const std::vector<std::string>& key_list,
								std::map<std::string, int64_t>& freq_map_) {
			std::map<std::string, int64_t>::iterator iter;
			for (int i = 0; i < (int)key_list.size(); i++) {
				const std::string key = key_list[i];
				int key_len = key.size();
				// Get substring frequency
				for (int j = 0; j < key_len; j++) {
					for (int k = 1; k <= key_len - j; k++) {
						std::string substring = key.substr(j, k);
						iter = freq_map_.find(substring);
						if (iter == freq_map_.end())
							freq_map_.insert(std::pair<std::string, int64_t>(substring, 1));
						else
							iter->second += 1;
					}
				}
			}
		}

		TEST_F (BlendTrieTest, baseNodeTest) {
			TrieNode node;
			node.setFreq(100);
			ASSERT_TRUE(node.getFreq() == 100);
			node.setPrefix("test");
			ASSERT_TRUE(node.getPrefix().compare("test") == 0);
			TrieNode child1;
			child1.setPrefix("1");
			TrieNode child2;
			child2.setPrefix("2");
			node.addChild('1', &child1);
			node.addChild('2', &child2);
			ASSERT_TRUE(node.getChild('2')->second->getPrefix().compare("2") == 0);
		}

		TEST_F (BlendTrieTest, smallTrie) {
			BlendTrie tree;
			std::map<std::string, int64_t> freq_map_;
			freq_map_.insert(std::make_pair("a", 1));
			freq_map_.insert(std::make_pair("b", 1));
			freq_map_.insert(std::make_pair("d", 1));
			freq_map_.insert(std::make_pair("ab", 1));
			freq_map_.insert(std::make_pair("bd", 1));
			freq_map_.insert(std::make_pair("abd", 1));
			tree.build(freq_map_);
			std::vector<SymbolFreq> freq_vec;
			tree.blendingAndGetLeaves(freq_vec);
			// for (auto it = freq_vec.begin(); it != freq_vec.end(); it++){
			// 	std::cout << it->first << "\t" << it->second << std::endl;
			// }
		}

		TEST_F (BlendTrieTest, emailTrie) {
			getFrequencyTable(emails, blend_freq_map);
			BlendTrie tree;
			tree.build(blend_freq_map);
			tree.blendingAndGetLeaves(blend_freq_vec);
			std::sort(blend_freq_vec.begin(), blend_freq_vec.end(),
				[](SymbolFreq& x, SymbolFreq& y) {
					return x.first.compare(y.first) < 0;
				});
			// tree.vis("../../datasets/vis_" + std::to_string(kEmailTestSize) + ".gvs");

			// compare result to those generated by python
			std::sort(blend_results.begin(), blend_results.end(),
				[](const SymbolFreq& x, const SymbolFreq& y) {
					return x.first.compare(y.first) < 0;
				});
	
			int cnt = 0;
			auto result_iter = blend_results.begin();
			for(auto iter = blend_freq_vec.begin(); iter != blend_freq_vec.end(); iter++) {

				ASSERT_TRUE(iter->first.compare(result_iter->first) == 0);
				ASSERT_TRUE(iter->second == result_iter->second);
				cnt += iter->second - result_iter->second;
				result_iter++;
			}
			ASSERT_TRUE(result_iter == blend_results.end());
		}

		void loadEmails() {
			std::ifstream infile(kEmailFilePath);
			std::string key;
			int count = 0;
			while (infile.good() && count < kEmailTestSize) {
				infile >> key;
				emails.push_back(key);
				count++;
			}
		}

		void loadBlendResult() {
			std::ifstream infile(kBlendingResultPath);
			std::string key;
			int64_t count = 0;
			int line_num = 0;
			while (infile >> key >> count) {
				blend_results.push_back(std::make_pair(key, count));
				line_num++;
			}
		}

	} // namespace blendtrietest

} // namespace ope

int main (int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);

	ope::blendtrietest::loadEmails();
	ope::blendtrietest::loadBlendResult();

	return RUN_ALL_TESTS();
}