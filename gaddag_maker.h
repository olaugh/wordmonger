#ifndef GADDAG_MAKER_H
#define GADDAG_MAKER_H

#include "fixed_string.h"
#include "util.h"

using std::map;
using std::vector;

#define DELIMITER 0
#define FIRST_LETTER 1

class GaddagMaker {
 public:
  GaddagMaker();
  bool MakeGaddag(const QString& input_path,
                  const QString& output_path);

 private:
  class Node {
   public:
    void PushWord(const WordString& word);
    int GetDepth();
    const QByteArray& GetHash();
    bool SameAs(const Node& other) const;
    void Number(int* bitsets, int* indices);
    QByteArray GetBytes(int num_child_bytes, int num_index_bytes) const;
    static void BinByHash(Node* node, map<QByteArray, vector<Node*>>* by_hash);
    static void MarkDuplicates(const map<QByteArray, vector<Node*>>& by_hash);

    Letter c;
    bool terminates;
    vector<Node> children;
    Node* duplicate;
    int bitsets;
    int indices;
    int depth = -1;
    QByteArray hash;
  };

  bool GaddagizeWord(const QString& word);
  void GaddagizeWord(const WordString &word);
  void HashWord(const WordString& word);
  void Generate();
  bool Write(const QString& output_path);
  void Write(const Node& node, int num_child_bytes, int num_index_bytes,
             QFile* ouputt);
  Node root;
  vector<WordString> gaddag_patterns;
  union {
    char charptr[16];
    int32_t int32ptr[4];
  } hash;
};

#endif // GADDAG_MAKER_H
