#include "search_server.h"
#include "iterator_range.h"
#include "profile.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

vector<string> SplitIntoWords(const string &line)
{
  istringstream words_input(line);
  return { istream_iterator<string>(words_input), istream_iterator<string>() };
}

SearchServer::SearchServer(istream &document_input)
{
  UpdateDocumentBase(document_input);
}

void SearchServer::UpdateDocumentBase(istream &document_input)
{
  InvertedIndex new_index;
  // Обработка в отдельных потоках?
  for (string current_document; getline(document_input, current_document);) {
    new_index.Add(move(current_document));
  }

  index = move(new_index);
}

void SearchServer::AddQueriesStream(
  istream &query_input,
  ostream &search_results_output)
{
#ifdef PERF_TEST
  TotalDuration lookup("Total lookup");
#endif

  for (string current_query; getline(query_input, current_query);) {
    vector<size_t> v_docid_cnt(50000);
    vector<size_t> docs_idxs(50000);
    size_t last_idx = 0;
    {
      for (const auto &word : SplitIntoWords(current_query)) {
#ifdef PERF_TEST
        ADD_DURATION(lookup);
#endif
        for (const auto &[docid, cnt] : index.Lookup(word)) {
          if (v_docid_cnt[docid] == 0) {
            docs_idxs[last_idx++] = docid;
          }
          v_docid_cnt[docid] += cnt;
        }
      }
    }

    vector<pair<size_t, size_t>> search_results;
    for (size_t docid = 0; docid < last_idx; docid++) {
      search_results.emplace_back(docs_idxs[docid], v_docid_cnt[docs_idxs[docid]]);
    }

    {
      std::partial_sort(
        begin(search_results), begin(search_results) + std::min<size_t>(5, search_results.size()), end(search_results), [](pair<size_t, size_t> lhs, pair<size_t, size_t> rhs) {
          int64_t lhs_docid = lhs.first;
          auto lhs_hit_count = lhs.second;
          int64_t rhs_docid = rhs.first;
          auto rhs_hit_count = rhs.second;
          return make_pair(lhs_hit_count, -lhs_docid) > make_pair(rhs_hit_count, -rhs_docid);
        });
    }

    search_results_output << current_query << ':';
    for (auto [docid, hitcount] : Head(search_results, 5)) {

      search_results_output << " {"
                            << "docid: " << docid << ", "
                            << "hitcount: " << hitcount << '}';
    }
    search_results_output << endl;
  }
}

void InvertedIndex::Add(const string &&document)
{
  docs.push_back(std::move(document));
  const size_t docid = docs.size() - 1;

  for (const auto &word : SplitIntoWords(docs.back())) {
    auto &vec = index[word];
    if (!vec.empty() && vec.back().first == docid) {
      vec.back().second += 1;
    } else {
      vec.emplace_back(docid, 1);
    }
  }
}

const vector<pair<size_t, size_t>> InvertedIndex::Lookup(const string &word) const
{
  if (auto it = index.find(word); it != index.end()) {
    return it->second;
  } else {
    return {};
  }
}
