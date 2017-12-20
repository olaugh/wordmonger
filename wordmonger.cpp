#include <QFile>
#include <QGridLayout>
#include <QtGui>
#include <QtWidgets>

#include "wordmonger.h"

Wordmonger* Wordmonger::m_self = 0;
Wordmonger* Wordmonger::self() { return m_self; }
Wordmonger::~Wordmonger() {}

Wordmonger::Wordmonger(QWidget* parent) : QMainWindow(parent) {
  m_self = this;

  QWidget* central_widget = new QWidget(this);

  QVBoxLayout* central_layout = new QVBoxLayout(central_widget);

  setCentralWidget(central_widget);

  loadDictionary("/Users/johnolaughlin/scrabble/csw15.txt", &csw);
  loadDictionary("/Users/johnolaughlin/scrabble/twl15.txt", &twl);

  loadWords();
  questions_layout = new QGridLayout;
  addQuestions();
  central_layout->addLayout(questions_layout);

  answer_line_edit = new QLineEdit(central_widget);
  answer_line_edit->setAlignment(Qt::AlignHCenter);
  QFont font("Futura", 18);
  answer_line_edit->setFont(font);
  QObject::connect(answer_line_edit, SIGNAL(textChanged(QString)), this,
                   SLOT(textChangedSlot(QString)));

  central_layout->addWidget(answer_line_edit);

  central_layout->setStretchFactor(questions_layout, 3);

  setMinimumSize(640, 360);
  resize(1080, 602);
}

void Wordmonger::resizeEvent(QResizeEvent* event) {
  QLinearGradient linearGrad(0, 0, event->size().width(), event->size().height());
  linearGrad.setColorAt(0, QColor(202, 210, 220));
  linearGrad.setColorAt(0.5, QColor(210, 221, 225));
  linearGrad.setColorAt(1, QColor(201, 207, 216));

  QPalette pal = palette();
  QBrush brush(linearGrad);
  pal.setBrush(QPalette::Window, brush);
  setAutoFillBackground(true);
  setPalette(pal);

  int v_spacing = std::min(9, height() / 72);
  int h_spacing = std::min(9, width() / 120);
  const int spacing = std::min(v_spacing, h_spacing);
  questions_layout->setHorizontalSpacing(spacing);
  questions_layout->setVerticalSpacing(spacing);

  qInfo() << "questions_layout->horizonalSpacing(): "
          << questions_layout->horizontalSpacing();

  const int line_edit_font_size = std::min(18, std::max(12, height() / 35));
  QFont font("Futura", line_edit_font_size);
  qInfo() << "line_edit_font_size: " << line_edit_font_size;
  answer_line_edit->setFont(font);
}

QString QuestionAndAnswer::getClue() const {
  std::map<QChar, int> counts;
  for (const QChar& c : clue) {
    counts[c]++;
  }
  QString vowels;
  QString consonants;
  for (const auto& pair : counts) {
    const QChar& c = pair.first;
    for (int i = 0; i < pair.second; ++i) {
      if (c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == "U") {
        vowels += c;
      } else {
        consonants += c;
      }
    }
  }
  return vowels + consonants;
}

void Wordmonger::addQuestions() {
 int i = 0;
  for (int col = 0; col < 5; col++) {
    for (int row = 0; row < 10; row++) {
      Question* question = new Question(this, i);
      questions.push_back(question);
      questions_layout->addWidget(question, row, col, 1, 1);
      for (const QString& answer : questions_and_answers[i].getAnswers()) {
        answer_map[answer].push_back(question);
      }
      i++;
    }
  }
}

Question::Question(QWidget* parent, int index) { this->index = index; }

void Question::markAnswer(const QString& given_answer) {
  qInfo() << "markAnswer(" << given_answer << ")...";
  const QuestionAndAnswer& q_and_a =
      Wordmonger::self()->questionAndAnswer(index);
  int answer_index = 0;
  for (const QString answer : q_and_a.getAnswers()) {
    if (answer == given_answer) {
      qInfo() << "solved answer #" << answer_index;
      solved_answer_indices.insert(answer_index);
      answer_index++;
    }
  }
  update();
}

void Question::paintEvent(QPaintEvent* event) {
  const QuestionAndAnswer& q_and_a =
      Wordmonger::self()->questionAndAnswer(index);
  const QString clue = q_and_a.getClue();
  QPainter painter;
  painter.begin(this);
  if (solved_answer_indices.size() == 0) {
    int clue_font_size = 0.5 * height();
    painter.fillRect(event->rect(), QColor(255, 255, 255, 64));
    QFont font("Futura", clue_font_size);
    QFontMetrics fm(font);
    int text_width = fm.width(clue);
    int text_height = fm.height();
    int descent = fm.descent();
    const int max_text_width = 0.8 * width();
    if (text_width > max_text_width) {
      clue_font_size = max_text_width * clue_font_size / text_width;
      clue_font_size = std::max(6, clue_font_size);
      QFont smaller_font("Futura", clue_font_size);
      QFontMetrics fm(smaller_font);
      text_width = fm.width(clue);
      text_height = fm.height();
      descent = fm.descent();
    }
    painter.setFont({"Futura", clue_font_size});

    int x = 0.5 * (width() - text_width);
    int y = 0.5 * (height() + text_height) - descent;
    painter.drawText(x, y, clue);
  } else {
    int answer_font_size = 0.45 * height();
    painter.eraseRect(event->rect());
    painter.fillRect(event->rect(), QColor(0, 0, 0, 20));
    QRect rect(0, 0, width() - 1, height() - 1);
    painter.setPen({0, 0, 0, 96});
    painter.drawRect(rect);
    const QString& answer = q_and_a.getAnswers()[0];
    QFont font("Futura", answer_font_size);
    QFontMetrics fm(font);
    int text_width = fm.width(answer);
    int text_height = fm.height();
    int descent = fm.descent();
    const int max_text_width = 0.8 * width();
    if (text_width > max_text_width) {
      answer_font_size = max_text_width * answer_font_size / text_width;
      answer_font_size = std::max(6, answer_font_size);
      QFont smaller_font("Futura", answer_font_size);
      QFontMetrics fm(smaller_font);
      text_width = fm.width(clue);
      text_height = fm.height();
      descent = fm.descent();
    }

    painter.setFont({"Futura", answer_font_size});
    if (Wordmonger::self()->IsTwl(answer)) {
      painter.setPen({0, 0, 0, 255});
    } else {
      painter.setPen({200, 0, 0, 255});
    }

    int x = 0.5 * (width() - text_width);
    int y = 0.5 * (height() + text_height) - descent;
    painter.drawText(x, y, answer);
  }
  painter.end();
}

void Wordmonger::loadWords() {
  QFile input("/Users/johnolaughlin/scrabble/csw-single-7s.txt");
  int i = 0;
  if (input.open(QIODevice::ReadOnly)) {
    QTextStream in(&input);
    while (!in.atEnd() && i < 50) {
      QString word = in.readLine();
      qInfo() << "word: " << word;
      QuestionAndAnswer q_and_a(word, {word});
      questions_and_answers.push_back(q_and_a);
      ++i;
    }
    input.close();
  }
  qInfo() << "#words: " << i;
}

void Wordmonger::loadDictionary(const QString& path, std::set<QString>* dict) {
  QFile input(path);
  if (input.open(QIODevice::ReadOnly)) {
    QTextStream in(&input);
    while (!in.atEnd()) {
      QString word = in.readLine();
      dict->insert(word);
    }
    input.close();
  }
  qInfo() << "loaded " << dict->size() << " from " << path;
}

QuestionAndAnswer::QuestionAndAnswer(const QString& clue,
                                     const std::vector<QString>& answers) {
  this->clue = clue;
  this->answers = answers;
}
