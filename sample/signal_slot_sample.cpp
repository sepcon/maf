#include <maf/Messaging.h>
#include <maf/Observable.h>
#include <maf/messaging/SignalTimer.h>
#include <maf/utils/TimeMeasurement.h>
#include <maf/utils/serialization/DumpHelper.h>

#include <iostream>
#include <map>

using namespace maf;
using namespace maf::util;
using namespace maf::signal_slots;
using namespace maf::messaging;
using namespace std;

static Signal<string> logMsg;
void initLogging() {
  logMsg.connect([](const string& msg) { cout << msg << endl; });
}

class Library;
class Account {
  struct Subscription {
    ScopedConnection connection;
    string bookName;
  };

  Library* lib = nullptr;
  string name;
  vector<Subscription> subscriptions;

 public:
  Account(string n, Library* l) : lib(l), name(move(n)) {}
  void subscribe_n(vector<string> books);
  void unsubcribe(initializer_list<string> books);
  void subscribe(const string& book);
  void unsubcribe(const string& book);
  void returnBook(const string& book);
  void onBookAvailable(const string& book) const;
};

class Library {
 public:
  using BookName = string;
  using BooksMap = map<string, size_t>;

  void returnBook(const BookName& name);
  void add(const BookName& name, size_t count = 1);
  void restore(const BookName& name, size_t count = 1);
  void save(const BookName& name, size_t count);
  void add(BooksMap books);
  size_t borrow(const BookName& name);
  Connection subscribe(Account* acc, const BookName& name);
  Account createAccount(string name);
  Observable<BooksMap> availableBooks;
};

void Library::returnBook(const Library::BookName& name) { restore(name); }

void Library::add(const Library::BookName& name, size_t count) {
  logMsg("Library added " + std::to_string(count) + " books of [" + name + "]");
  save(name, count);
}

void Library::restore(const Library::BookName& name, size_t count) {
  logMsg("Library restored " + std::to_string(count) + " books of [" + name +
         "]");
  save(name, count);
}

void Library::save(const Library::BookName& name, size_t count) {
  (*availableBooks.mut())[name] += count;
}

void Library::add(Library::BooksMap books) {
  logMsg("[ANOUNCEMENT] added new books: " + srz::dump(books));
  availableBooks.mut()->merge(books);
}

size_t Library::borrow(const Library::BookName& name) {
  auto mBooks = availableBooks.mut();
  if (auto& bookCount = (*mBooks)[name]; bookCount > 0) {
    --bookCount;
    return 1;
  } else {
    return 0;
  }
}

Connection Library::subscribe(Account* acc, const Library::BookName& name) {
  return availableBooks.connect(
      [name, this, acc](auto connectionPtr, const Library::BooksMap& books) {
        try {
          if (books.at(name) > 0) {
            connectionPtr->disconnect();
            borrow(name);
            acc->onBookAvailable(name);
          }
        } catch (...) {
        }
      });
}

Account Library::createAccount(string name) {
  return Account{move(name), this};
}

int main() {
  initLogging();
  Library lib;

  lib.add(
      {{"hello", 1}, {"world", 1}, {"goodbye", 1}, {"goodbye", 1}, {"you", 5}});

  auto user1 = lib.createAccount("user1");
  auto user2 = lib.createAccount("user2");
  auto user3 = lib.createAccount("user3");
  user1.subscribe_n({"hello", "world", "later", "later1"});
  user2.subscribe_n({"hello", "world", "later", "later1", "no1"});
  user3.subscribe("hello");
  user1.returnBook("world");
  user1.returnBook("hello");

  lib.add("no1", 2);
  lib.add("later1", 2);
  user3.subscribe_n({"later1", "no1"});

  return 0;
}

void Account::subscribe_n(vector<string> books) {
  for (auto& book : books) {
    subscribe(book);
  }
}

void Account::unsubcribe(initializer_list<string> books) {
  for (auto& book : books) {
    unsubcribe(book);
  }
}

void Account::subscribe(const string& book) {
  logMsg("user: " + name + " is subscribing book [" + book + "]");
  lib->subscribe(this, book);
}

void Account::unsubcribe(const string& book) {
  for (auto it = subscriptions.begin(); it != subscriptions.end(); ++it) {
    if (it->bookName == book) {
      subscriptions.erase(it);
      break;
    }
  }
}

void Account::returnBook(const string& book) {
  logMsg("User " + name + " returns book " + book);
  lib->returnBook(book);
  unsubcribe(book);
}

void Account::onBookAvailable(const string& book) const {
  logMsg(name + " got book: [" + book + "]");
}
