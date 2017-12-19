#ifndef WORDMONGER_H
#define WORDMONGER_H

#include <QDebug>
#include <QGridLayout>
#include <QMainWindow>
#include <QObject>
#include <QLineEdit>
#include <QWidget>

#include <set>
#include <vector>

class QLineEdit;

class Question : public QWidget {
 public:
  Question(QWidget* parent = 0);
  Question(QWidget* parent = 0, int index = -1);
  void markAnswer(const QString& answer);

 protected:
  void paintEvent(QPaintEvent* event);

 private:
  int index;
  std::set<int> solved_answer_indices;
};

class QuestionAndAnswer {
 public:
  QuestionAndAnswer(const QString& clue, const std::vector<QString>& answers);
  QString getClue() const;
  const std::vector<QString>& getAnswers() const {
    return answers;
  }

 private:
  QString clue;
  std::vector<QString> answers;
};


class Wordmonger : public QMainWindow {
  Q_OBJECT

 public:
  Wordmonger(QWidget* parent);
  static Wordmonger* self();

  const QuestionAndAnswer& questionAndAnswer(int i) const {
    return questions_and_answers[i];
  }
  bool IsTwl(QString word) const {
    return twl.count(word) > 0;
  }

  ~Wordmonger();

 public slots:
  void textChangedSlot(QString text) {
    qInfo() << "text: " << text;
    QString uppercase_text = text.toUpper();
    if (text == uppercase_text) return;

    const int cursor_position = answer_line_edit->cursorPosition();
    answer_line_edit->setText(uppercase_text);
    answer_line_edit->setCursorPosition(cursor_position);

    auto it = answer_map.find(uppercase_text);
    if (it != answer_map.end()) {
      for (Question* question : it->second) {
        question->markAnswer(uppercase_text);
      }
      answer_line_edit->setText("");
    }
  }

 protected:
  void resizeEvent(QResizeEvent* event) override;

 private:
  void loadWords();
  std::vector<QuestionAndAnswer> questions_and_answers;
  void addQuestions();
  QGridLayout* questions_layout;
  std::vector<Question*> questions;
  std::map<QString, std::vector<Question*>> answer_map;

  void loadDictionary(const QString& path, std::set<QString>* dict);
  std::set<QString> twl;
  std::set<QString> csw;

  QLineEdit* answer_line_edit;

  static Wordmonger* m_self;
};

#endif  // WORDMONGER_H
