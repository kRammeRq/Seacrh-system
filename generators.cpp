#include "generators.h"
#include "parse.h"
#include <fstream>
#include <stdlib.h>
#include <string>
#include <vector>

std::vector<std::string> read_words(const std::string path, int top)
{
  std::vector<std::string> words;
  words.reserve(top);

  std::ifstream ifs;
  ifs.open(path);
  while (top--) {
    std::string word;
    ifs >> word;
    words.push_back(word);
  }
  ifs.close();
  return words;
}

std::string generate_doc(const std::vector<std::string> words, int lenght = 100)
{
  std::vector<std::string> random_words;
  random_words.reserve(lenght);
  for (int i = 0; i < lenght; i++) {
    random_words.push_back(words[rand() % lenght]);
  }
  return Join(' ', random_words);
}

std::vector<std::string> generateDocs(int cnt)
{
  std::vector<std::string> words = read_words("english_top10k_from_google.txt", 300);

  std::vector<std::string> docs;
  docs.reserve(cnt);
  while (cnt--) {
    docs.push_back(generate_doc(words));
  }
  return docs;
}

std::string generate_query(const std::vector<std::string> words, int lenght = 10)
{
  std::vector<std::string> random_words;
  random_words.reserve(lenght);
  for (int i = 0; i < lenght; i++) {
    random_words.push_back(words[rand() % lenght]);
  }
  return Join(' ', random_words);
}

std::vector<std::string> generateQueries(int cnt)
{
  std::vector<std::string> words = read_words("english_top10k_from_google.txt", 100);
  std::vector<std::string> queries;
  queries.reserve(cnt);
  while (cnt--) {
    queries.push_back(generate_query(words, 1 + rand() % 10));
  }
  return queries;
}