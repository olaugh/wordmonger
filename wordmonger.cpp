#include <QFile>
#include <QGridLayout>
#include <QtGui>
#include <QtWidgets>

#include "gaddag_maker.h"
#include "wordmonger.h"


Wordmonger* Wordmonger::self = 0;
Wordmonger* Wordmonger::get() { return self; }
Wordmonger::~Wordmonger() {}

Wordmonger::Wordmonger(QWidget* parent) : QMainWindow(parent) {
  self = this;

  LoadDictionaries();
  CreateWidgets();
  ChooseWords();
  AddQuestions();
  StartTimer();
}

void Wordmonger::CreateWidgets() {
  QWidget* central_widget = new QWidget(this);
  QVBoxLayout* central_layout = new QVBoxLayout(central_widget);
  central_layout->setContentsMargins(0, 0, 0, 0);
  central_layout->setSpacing(0);
  setCentralWidget(central_widget);

  questions_layout = new QGridLayout;
  central_layout->addLayout(questions_layout);

  answer_line_edit = new QLineEdit(central_widget);
  answer_line_edit->setAlignment(Qt::AlignHCenter);
  QObject::connect(answer_line_edit, SIGNAL(textChanged(QString)), this,
                   SLOT(textChangedSlot(QString)));

  central_layout->addWidget(answer_line_edit);

  word_status_bar = new WordStatusBar(this);
  central_layout->addWidget(word_status_bar);

  central_layout->setStretchFactor(questions_layout, 3);

  setMinimumSize(640, 360);
  resize(1080, 602);
}

void Wordmonger::StartTimer() {
  timer_millis = 180 * 1000;
  elapsed_timer.start();
  timer.start(1000 / 60, this);
  time_expired = false;
  quiz_finished = false;
}

void Wordmonger::LoadDictionaries() {
  LoadDictionary("/Users/johnolaughlin/scrabble/csw15.txt", &csw);
  LoadDictionary("/Users/johnolaughlin/scrabble/twl15.txt", &twl);

  GaddagMaker gaddag_maker;
  gaddag_maker.MakeGaddag("/Users/johnolaughlin/scrabble/csw15.txt",
                          "/Users/johnolaughlin/scrabble/csw15.gaddag");
}

void Wordmonger::timerEvent(QTimerEvent *event) {
  if (event->timerId() !=timer.timerId()) {
    QWidget::timerEvent(event);
    return;
  }
  if (time_expired || quiz_finished) {
    return;
  }
  timer_millis -= elapsed_timer.restart();
  //int radius = (180 * 1000 - timer_millis) / 1000;
  if (timer_millis <= 0) {
      time_expired = true;
      quiz_finished = true;
      timer_millis = 0;
      for (Question* q : questions) {
        //q->SetBlurRadius(radius);
        q->update();
      }
  }
  //qInfo() << "timer_millis: " << timer_millis;
  word_status_bar->SetTime(timer_millis);
  word_status_bar->update();
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
  questions_layout->setContentsMargins(spacing, spacing, spacing, spacing);
  questions_layout->setHorizontalSpacing(spacing);
  questions_layout->setVerticalSpacing(spacing);

  qInfo() << "questions_layout->horizonalSpacing(): "
          << questions_layout->horizontalSpacing();

  const int line_edit_font_size = std::min(18, std::max(12, height() / 35));
  QFont font("Futura", line_edit_font_size);
  qInfo() << "line_edit_font_size: " << line_edit_font_size;
  answer_line_edit->setFont(font);

  const int status_bar_font_size = std::min(16, std::max(10, height() / 42));
  qInfo() << "status_bar_font_size: " << status_bar_font_size;
  word_status_bar->SetFontSize(status_bar_font_size);
  word_status_bar->setMinimumHeight(line_edit_font_size * 1.75);
}

namespace {
  QString alphagram(const QString& word) {
    std::map<QChar, int> counts;
    for (const QChar& c : word) {
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
}  // namespace

QString QuestionAndAnswer::GetClue() const {
  return alphagram(clue);
}

void Wordmonger::AddQuestions() {
 int i = 0;
 for (int col = 0; col < 5; col++) {
   for (int row = 0; row < 9;) {
     const std::vector<QString>& answers =
         questions_and_answers[i].GetAnswers();
     if (answers.size() + row > 9) {
       break;
     }
     Question* question = new Question(this, i);
     questions.push_back(question);
     questions_layout->addWidget(question, row, col, answers.size(), 1);
     for (const QString& answer : answers) {
       answer_map[answer].push_back(question);
     }
     row += answers.size();
     i++;
   }
  }
}

void Wordmonger::CheckIfQuizFinished() {
  for (Question* question : questions) {
    if (!question->IsAllSolved()) {
      return;
    }
  }
  quiz_finished = true;
  word_status_bar->update();
}

WordStatusBar::WordStatusBar(QWidget *parent) {}

void WordStatusBar::paintEvent(QPaintEvent* event) {
  QPainter painter;
  painter.begin(this);
  painter.eraseRect(event->rect());

  double time_seconds = time_millis / 1000.0;
  const int decimal_places =
    (Wordmonger::get()->QuizFinished() || time_seconds < 10) ? 1 : 0;
  QString time_string = QString::number(time_seconds, 'f', decimal_places);

  QFont font("Futura", font_size);
  QFontMetrics fm(font);
  int text_width = fm.width(time_string);
  int text_height = fm.height();
  int descent = fm.descent();
  painter.setFont(font);

  int x = 0.5 * (width() - text_width);
  int y = 0.5 * (height() + text_height) - descent;
  painter.drawText(x, y, time_string);
}

Question::Question(QWidget* parent, int index) {
  this->index = index;
  blur_effect = new QGraphicsBlurEffect(this);
  blur_effect->setBlurRadius(0);
  setGraphicsEffect(blur_effect);
}

bool Question::IsAllSolved() const {
  const QuestionAndAnswer& q_and_a =
    Wordmonger::get()->QuestionAndAnswerAt(index);
  return solved_answer_indices.size() == q_and_a.GetAnswers().size();
}

void Question::MarkAnswer(const QString& given_answer) {
  qInfo() << "MarkAnswer(" << given_answer << ")...";
  const QuestionAndAnswer& q_and_a =
      Wordmonger::get()->QuestionAndAnswerAt(index);
  int answer_index = 0;
  for (const QString answer : q_and_a.GetAnswers()) {
    qInfo() << "answer: " << answer;
    if (answer == given_answer) {
      qInfo() << "solved answer #" << answer_index;
      solved_answer_indices.insert(answer_index);
    }
    answer_index++;
    qInfo() << "new answer_index: " << answer_index;
  }
  Wordmonger::get()->CheckIfQuizFinished();
  update();
}

void Question::paintEvent(QPaintEvent* event) {
  const QuestionAndAnswer& q_and_a =
      Wordmonger::get()->QuestionAndAnswerAt(index);
  const QString clue = q_and_a.GetClue();
  const bool quiz_finished = Wordmonger::get()->QuizFinished();
  QPainter painter;
  painter.begin(this);
  painter.eraseRect(event->rect());
  int clue_height = height();
  const auto& answers = q_and_a.GetAnswers();
  int num_subcells = 1;
  const bool partially_revealed =
      !quiz_finished && (solved_answer_indices.size() > 0) &&
      (solved_answer_indices.size() < answers.size());
  const bool wrong = quiz_finished &&
      (solved_answer_indices.size() < answers.size());
  //qInfo() << "partially_revealed: " << partially_revealed;
  if (solved_answer_indices.size() == 0 && !quiz_finished) {
    num_subcells = 1;
  } else if (partially_revealed) {
    num_subcells = answers.size() + 1;
  } else {
    num_subcells = answers.size();
  }
  //qInfo() << "num_subcells: " << num_subcells;
  std::vector<int> fill_dividing_line_heights;
  std::vector<int> border_dividing_line_heights;
  for (int i = 0; i <= num_subcells; i++) {
    int y = (i * height() - 1) / num_subcells;
    fill_dividing_line_heights.push_back(y);
    if (i == num_subcells) {
      y--;
    }
    border_dividing_line_heights.push_back(y);
  }
  clue_height = fill_dividing_line_heights[1] - fill_dividing_line_heights[0];
  int clue_font_size = 0.5 * clue_height;
  int answer_font_size = 0.45 * clue_height;
  if (!quiz_finished && (solved_answer_indices.size() == 0 ||
                         solved_answer_indices.size() < answers.size())) {
    QRect rect(0, 0, width(), clue_height + 1);
    painter.fillRect(rect, QColor(255, 255, 255, 80));

    if (solved_answer_indices.size() > 0) {
      QRect border_rect(0, 0, width() - 1, clue_height);
      painter.setPen({0, 0, 0, 32});
      painter.drawRect(border_rect);
    }

    QFont font("Futura", clue_font_size);
    QFontMetrics fm(font);
    int text_width = fm.width(clue);
    int text_height = fm.height();
    int descent = fm.descent();
    const int max_text_width = 0.75 * width();
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
    int y = 0.5 * (clue_height + text_height) - descent;
    painter.drawText(x, y, clue);
  }
  if (!solved_answer_indices.empty() || quiz_finished) {
    for (unsigned int i = 0; i < answers.size(); ++i) {
      const int top_dividing_index =
          partially_revealed ? i + 1 : i;
      const int bottom_dividing_index = top_dividing_index + 1;
      const int top = fill_dividing_line_heights[top_dividing_index];
      const int bottom = fill_dividing_line_heights[bottom_dividing_index];
      const int border_bottom =
          border_dividing_line_heights[bottom_dividing_index];
      //qInfo() << "i: " << i << " bottom: " << bottom << " top: " << top;
      const int answer_height = bottom - top;
      const int answer_border_height = border_bottom - top;
      QRect rect(0, top + 1, width() - 1, answer_height);
      QRect border_rect(0, top + 1, width() - 1, answer_border_height);
      //qInfo() << "i: " << i << ", rect({" << rect.left() << ", " << rect.bottom()
      //        << ", " << rect.right() << ", " << rect.top() << "})";
      if (solved_answer_indices.count(i) == 0) {
        painter.fillRect(rect, QColor(255, 255, 255, 48));
      } else {
        painter.fillRect(rect, QColor(0, 0, 0, 20));
      }
      painter.setPen({0, 0, 0, 96});
      painter.drawRect(border_rect);

      const QString& answer = q_and_a.GetAnswers()[i];
      QString display_answer = answer;
      const bool solved_this_answer = solved_answer_indices.count(i) > 0;
      if (!quiz_finished && !solved_this_answer) {
        display_answer = "";
        for (int j = 0 ; j < answer.length();  j++) {
            display_answer += "–";
        }
      }
      //qInfo() << "painting text for answer: " << answer;
      QFont font("Futura", answer_font_size);
      QFontMetrics fm(font);
      int text_width = fm.width(display_answer);
      int text_height = fm.height();
      int descent = fm.descent();
      const int max_text_width = 0.75 * width();
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
      if (Wordmonger::get()->IsTwl(answer)) {
        painter.setPen({0, 0, 0, solved_this_answer ? 224 : 255});
      } else {
        painter.setPen({200, 0, 0, solved_this_answer ? 224 : 255});
      }

      int x = 0.5 * (width() - text_width);
      int y = top + 0.5 * (answer_height + text_height) - descent;
      painter.drawText(x, y, display_answer);
    }
  }
  if (wrong) {
    QPen pen({0, 0, 20, 196});
    painter.setPen(pen);
    for (int offset = 0; offset < 3; offset++) {
      QRect wrong_border(offset, offset,
                         width() - 1 - 2 * offset, height() - 1 - 2 * offset);
      painter.drawRect(wrong_border);
    }
  }
  painter.end();
}

void Wordmonger::ChooseWords() {
  std::map<QString, std::vector<QString>> sets;
  for (const QString& word : csw) {
    if (word.length() != 7) {
      continue;
    }
    const QString alpha = alphagram(word);
    sets[alpha].push_back(word);
  }

  std::vector<std::pair<QString, std::vector<QString>>> pairs(
        sets.begin(), sets.end());
  std::random_shuffle(pairs.begin(), pairs.end());
  int i = 0;
  for (const auto& pair : pairs) {
    if (pair.second.size() != 1) {
      continue;
    }
    if (i >= 45) {
      return;
    }
    i++;
    QuestionAndAnswer q_and_a(pair.first, pair.second);
    questions_and_answers.push_back(q_and_a);
  }
}

void Wordmonger::LoadSingleAnagramWords() {
  QFile input("/Users/johnolaughlin/scrabble/csw-single-7s.txt");
  int i = 0;
  if (input.open(QIODevice::ReadOnly)) {
    QTextStream in(&input);
    while (!in.atEnd() && i < 45) {
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

void Wordmonger::LoadDictionary(const QString& path, std::set<QString>* dict) {
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
