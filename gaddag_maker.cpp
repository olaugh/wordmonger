#include <bitset>
#include <iostream>

#include <QByteArray>
#include <QtCore>
#include <QCryptographicHash>

#include "gaddag_maker.h"
#include "util.h"

constexpr int kGaddagVersion = 2;

GaddagMaker::GaddagMaker(bool make_dawg, bool flip_endian) {
  root.terminates = false;
  root.c = DELIMITER;
  this->make_dawg = make_dawg;
  this->flip_endian = flip_endian;
}

bool GaddagMaker::MakeGaddag(const QString& input_path,
                             const QString& output_path) {
  qInfo() << "input_path: " << input_path;
  qInfo() << "output_path: " << output_path;

  gaddag_patterns.clear();
  QFile input(input_path);
  if (input.open(QIODevice::ReadOnly)) {
    QTextStream in(&input);
    while (!in.atEnd()) {
      QString word = in.readLine();
      if (!GaddagizeWord(word)) {
        qInfo() << "Could not encode word " << word;
      }
    }
  } else {
    qInfo() << "could not open input file";
    return false;
  }
  Generate();
  Write(output_path);
  return true;
}

void GaddagMaker::HashWord(const WordString& word) {
  QCryptographicHash word_hash(QCryptographicHash::Md5);
  word_hash.addData(word.constData(), word.length());
  QByteArray hash_bytes = word_hash.result();
  hash.int32ptr[0] ^= ((const int32_t*)hash_bytes.constData())[0];
  hash.int32ptr[1] ^= ((const int32_t*)hash_bytes.constData())[1];
  hash.int32ptr[2] ^= ((const int32_t*)hash_bytes.constData())[2];
  hash.int32ptr[3] ^= ((const int32_t*)hash_bytes.constData())[3];
}

bool GaddagMaker::GaddagizeWord(const QString& word) {
  const WordString word_string = Util::EncodeWord(word);
  if (word_string.empty()) {
    return false;
  }
  HashWord(word_string);
  GaddagizeWord(word_string);
  return true;
}

void GaddagMaker::GaddagizeWord(const WordString& word) {
  if (make_dawg) {
    gaddag_patterns.push_back(word);
    return;
  }
  for (size_t switch_index = 0; switch_index <= word.size(); ++switch_index) {
    WordString pattern;
    for (int i = switch_index - 1; i >= 0; --i) {
      pattern.push_back(word[i]);
    }
    if (switch_index < word.size()) {
      pattern.push_back(DELIMITER);
      for (size_t i = switch_index; i < word.size(); ++i) {
        pattern.push_back(word[i]);
      }
    }
    gaddag_patterns.push_back(pattern);
  }
}

void GaddagMaker::Generate() {
  qInfo() << "we have" << gaddag_patterns.size() << "gaddag patterns";
  sort(gaddag_patterns.begin(), gaddag_patterns.end());
  qInfo() << "sorted them";
  for (const WordString& pattern : gaddag_patterns) {
    root.PushWord(pattern);
  }
}

void GaddagMaker::Node::PushWord(const WordString& word) {
  if (word.length() == 0) {
    terminates = true;
    return;
  }

  const Letter first = word[0];
  const WordString rest = word.substr(1, word.length() - 1);

  int index = -1;
  for (size_t i = 0; i < children.size(); i++) {
    if (children[i].c == first) {
      index = i;
      break;
    }
  }

  if (index < 0) {
    Node n;
    n.c = first;
    n.terminates = false;
    n.duplicate = nullptr;
    children.push_back(n);
    index = children.size() - 1;
  }

  children[index].PushWord(rest);
}

bool GaddagMaker::Write(const QString& output_path) {
  qInfo() << "writing to" << output_path;
  QFile output(output_path);

  if (output.open(QIODevice::WriteOnly)) {
    output.putChar(kGaddagVersion);
    output.write(hash.charptr, sizeof(hash.charptr));

    int bitsets = 0;
    int indices = 0;
    map<QByteArray, vector<Node*>> by_hash;
    Node::BinByHash(&root, &by_hash);
    Node::MarkDuplicates(by_hash);
    root.Number(&bitsets, &indices);

    const int alphabet_size = LAST_LETTER + 1;
    const int num_child_bytes = (alphabet_size + 8 - 1) / 8;
    qInfo() << "num_child_bytes: " << num_child_bytes;
    int num_index_bytes = 1;
    for (; num_index_bytes < 8; ++num_index_bytes) {
      int64_t bytes_addressable = 1L << (num_index_bytes * 8);
      if (bytes_addressable >
          2 * (num_child_bytes * bitsets + num_index_bytes * indices)) {
        break;
      }
    }
    qInfo() << "num_index_bytes: " << num_index_bytes;
    output.putChar(LAST_LETTER);
    output.putChar(num_child_bytes);
    output.putChar(num_index_bytes);
    Write(root, num_child_bytes, num_index_bytes, &output);
  }
  output.close();
  return true;
}

namespace {
inline void ULongToBytes(unsigned long ulong, int length, char* bytes,
                         bool flip_endian) {
  for (int i = 0; i < length; ++i) {
    const int shift = i * 8;
    int dest_i = flip_endian ? length - 1 - i : i;
    bytes[dest_i] = (ulong >> shift) & 0xFF;
  }
}
}  // namespace

QByteArray GaddagMaker::Node::GetBytes(int num_child_bytes,
                                       int num_index_bytes,
                                       bool flip_endian) const {
  QByteArray ret;
  int num_child_pointer_bytes = children.size() * num_index_bytes;
  char child_pointer_bytes[num_child_pointer_bytes];
  std::bitset<64> child_bits;
  int offset = 0;
  for (const Node& child : children) {
    const Node& child_for_pointer =
        (child.duplicate == nullptr) ? child : *child.duplicate;
    unsigned long child_index = num_child_bytes * child_for_pointer.bitsets +
                                num_index_bytes * child_for_pointer.indices;
    ULongToBytes(child_index, num_index_bytes, child_pointer_bytes + offset,
                 flip_endian);
    if (child.terminates) {
      // set most significant bit to mark termination;
      const int termination_byte_index = flip_endian ?
            0 : num_index_bytes - 1;
      child_pointer_bytes[offset + termination_byte_index] |= 0b10000000;
    }
    offset += num_index_bytes;
    Letter letter = child.c;
    child_bits.set(letter);
  }
  unsigned long child_bits_int = child_bits.to_ulong();
  char child_bytes[num_child_bytes];
  ULongToBytes(child_bits_int, num_child_bytes, child_bytes, flip_endian);
  ret.append(child_bytes, num_child_bytes);
  ret.append(child_pointer_bytes, num_child_pointer_bytes);
  return ret;
}

void GaddagMaker::Write(const Node& node, int num_child_bytes,
                        int num_index_bytes, QFile* output) {
  QByteArray bytes = node.GetBytes(num_child_bytes, num_index_bytes,
                                   flip_endian);
  output->write(bytes);
  for (const Node& child : node.children) {
    if (child.duplicate == nullptr && !child.children.empty()) {
      Write(child, num_child_bytes, num_index_bytes, output);
    }
  }
}

void GaddagMaker::Node::Number(int* bitsets, int* indices) {
  if (children.empty()) {
    this->bitsets = 0;
    this->indices = 0;
    return;
  }
  this->bitsets = *bitsets;
  this->indices = *indices;
  ++(*bitsets);
  (*indices) += children.size();
  for (Node& child : children) {
    if (child.duplicate == nullptr) {
      child.Number(bitsets, indices);
    }
  }
}

void GaddagMaker::Node::BinByHash(Node* node,
                                  map<QByteArray, vector<Node*>>* by_hash) {
  (*by_hash)[node->GetHash()].push_back(node);
  for (Node& child : node->children) {
    BinByHash(&child, by_hash);
  }
}

const QByteArray& GaddagMaker::Node::GetHash() {
  if (hash.isEmpty()) {
    QCryptographicHash h(QCryptographicHash::Md5);
    h.addData("foo");
    for (Node& node : children) {
      QByteArray nodec;
      nodec.append(node.c);
      h.addData(nodec);
      h.addData(node.terminates ? "." : "-");
      h.addData(node.GetHash());
    }
    hash = h.result();
  }
  return hash;
}

void GaddagMaker::Node::MarkDuplicates(
    const map<QByteArray, vector<Node*>>& by_hash) {
  for (const auto hash_pair : by_hash) {
    for (size_t i = 0; i < hash_pair.second.size(); ++i) {
      Node* node_i = hash_pair.second[i];
      if (node_i->duplicate != NULL) continue;
      for (unsigned int j = i + 1; j < hash_pair.second.size(); ++j) {
        Node* node_j = hash_pair.second[j];
        if (node_i->SameAs(*node_j)) {
          node_j->duplicate = node_i;
        } else {
          qInfo() << "collision... i: " << i << " j: " << j << endl;
        }
      }
    }
  }
}

bool GaddagMaker::Node::SameAs(const Node& other) const {
  if (children.size() != other.children.size()) return false;
  for (size_t i = 0; i < children.size(); ++i) {
    if (children[i].c != other.children[i].c) return false;
    if (children[i].terminates != other.children[i].terminates) return false;
  }
  for (size_t i = 0; i < children.size(); ++i) {
    if (!children[i].SameAs(other.children[i])) return false;
  }
  return true;
}
