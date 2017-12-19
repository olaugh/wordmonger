#include "wordmonger.h"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  Wordmonger w(nullptr);
  w.show();

  return a.exec();
}
