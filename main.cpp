#include <iostream>
#include <sstream>

#include "batcher.h"
template <typename T>
void Notifier(std::vector<T> batch) {
  std::cout << "string in batch" << std::endl;
  for (auto& it : batch) {
    std::cout << it << std::endl;
  }
}

void TestBatcher() {
  edk::Batcher<std::string> b;
  b.SetBatchSize(4);
  b.SetNotifier(Notifier<std::string>);
  b.AddItem("batch 1: first");
  b.AddItem("batch 1: second");
  b.AddItem("batch 1: third");
  b.AddItem("batch 1: fourth");

  b.AddItem("batch 2: first");
  b.AddItem("batch 2: second");
  b.AddItem("batch 2: third");
  b.AddItem("batch 2: fourth");

  edk::Batcher<int> bi;
  bi.SetBatchSize(4);
  bi.SetNotifier(Notifier<int>);
  bi.AddItem(1);
  bi.AddItem(2);
  bi.AddItem(3);
  bi.AddItem(4);

  bi.AddItem(5);
  bi.AddItem(6);
  bi.AddItem(7);
  bi.AddItem(8);
}

#include "item_container.h"

void TestSmartPtr() {
  ItemContainer ic;
}

void TestPrintPtr() {
  void* a = new int[5];
  std::ostringstream ss;
  ss << "pointer address: " << a;
  std::cout << ss.str() << std::endl;
}

void TestPrintBool() {
  std::cout << std::boolalpha;
  std::cout << true << '\n';
  std::cout << false << '\n';
}

#include "any.h"
#include "item.h"

void SomeFunc(any a) {
  std::cout << "################pass any to a func\n";
}

void TestAny() {
  std::cout << "test any" << std::endl;
  any aint(5);
  int a = any_cast<int>(aint);
  int b = any_cast<int>(aint);
  std::cout << a << "\t" << b << std::endl;

  OneItem i;
  i.data = "test any";
  std::cout << "@@@@@@@@@@@@@@@@before assign value to any\n";
  any aitem(i);
  SomeFunc(aitem);
  std::cout << "@@@@@@@@@@@@@@@@before get value from any\n";
  const auto& i1 = any_cast<OneItem>(i);
  std::cout << "--------------------------\n";
  auto i2 = any_cast<OneItem>(i);
  std::cout << "@@@@@@@@@@@@@@@@after get value from any\n";
  std::cout << "i: " << i.data << "\n";
  std::cout << "i1: " << i1.data << "\n";
  std::cout << "i2: " << i2.data << "\n";

  std::cout << "---------------\n";
  auto& lref = any_cast<OneItem&>(aitem);
  std::cout << "---------------\n";
}

#include <unistd.h>
#include <cerrno>
#include <cstring>
void TestExecutable() {
  int iret = execl("/bin/ls", "-asl", NULL);
  std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$" << iret << "\n";
  if (iret==-1) printf("%d:%s\n",errno,std::strerror(errno));
}

#include <curl/curl.h>
#include <sys/stat.h>
#include <cstdlib>

class CurlDownloader {
 public:
  explicit CurlDownloader(const std::string& model_dir): model_dir_(model_dir) {
    if (access(model_dir_.c_str(), W_OK) != 0) {
      std::cerr << "model directory not exist or do not have write permission: " << model_dir_ << "\n";
      std::exit(-1);
    }
    curl_ = curl_easy_init();
    curl_easy_setopt(curl_, CURLOPT_NOPROGRESS, false);
  }

  std::string Download(const std::string& url) {
    // get name of model file
    std::string file_path = model_dir_ + url.substr(url.find_last_of('/'));

    if (access(file_path.c_str(), F_OK) == 0) {
      std::cout << "model exists in specified directory, skip download\n";
      return file_path;
    } else {
      FileHandle f(file_path, "wb");
      FILE* file = f.GetFile();

      curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl_, CURLOPT_WRITEDATA, file);

      auto re = curl_easy_perform(curl_);
      if (re != CURLE_OK) {
        std::cerr << "Download model error, error_code: " << re << "\n";
        std::cerr << "model url: " << url << "\n";
        return {};
      }
      return file_path;
    }
  }

  ~CurlDownloader() {
    if (curl_) {
      curl_easy_cleanup(curl_);
      curl_ = nullptr;
    }
  }

 private:
  class FileHandle {
   public:
    explicit FileHandle(const std::string& fpath, const char* mode) {
      file_ = fopen(fpath.c_str(), mode);
    }
    FILE* GetFile() {return file_;}
    ~FileHandle() {
      fclose(file_);
    }
  
   private:
    FILE* file_ = nullptr;
  };  // class FileHandle

  std::string model_dir_;
  CURL* curl_ = nullptr;

};  // class CurlDownloader

bool BeginWith(const std::string& s, const std::string& prefix) {
  if (s.size() < prefix.size()) return false;
  return prefix == s.substr(0, prefix.size());
}

static inline bool IsNetFile(const std::string& url) {
  std::vector<std::string> protocols = {"http://", "https://", "ftp://"};
  bool is_net = false;
  for (auto& proto : protocols) {
    if (BeginWith(url, proto)) {
      is_net = true;
      break;
    }
  }
  return is_net;
}

void TestCurl() {
  CurlDownloader d("./");
  const char *url = "https://csdnimg.cn/cdn/content-toolbar/csdn-logo.png?v=20200416.1";
  const char *url_ftp = "ftp://www.baidu.csdg";
  const char *url_local = "/www/csdg";
  const char *url_wrong = "rrrrrrrr";
  std::cout << std::boolalpha << IsNetFile(url) << std::endl;
  std::cout << std::boolalpha << IsNetFile(url_ftp) << std::endl;
  std::cout << std::boolalpha << IsNetFile(url_local) << std::endl;
  std::cout << std::boolalpha << IsNetFile(url_wrong) << std::endl;
  /* const char *wrong_url = "https://www.tesgvsad.cn/gawi/gsd.txt"; */
  std::cout << "download file in: " << d.Download(url) << std::endl;
  /* std::cout << "download file in: " << d.Download(url) << std::endl; */
  /* std::cout << "download file in: " << d.Download(wrong_url) << std::endl; */
}

#include "temp.h"
void TestTemp() {
  aContainer a;
  int i_a;
  a.PrintTypeAddress(i_a);
  PrintIntTypeAddress();
}

#include <functional>
/* void InvokeFunc(std::shared_ptr<std::function<void()>> func) { */
void InvokeFunc(std::function<void()> func) {
  (func)();
};

template<typename ...Arg>
void DoNothing(Arg ...args) {
  /* (*func)(); */
  std::cout << "func: do nothing\n";
};
void TestFunctional() {
  std::cout << "test functional\n";
  OneItem it;
  /* auto func = std::make_shared<std::function<void()>>(std::bind([](OneItem it) {}, it)); */
  auto func = std::bind([](OneItem it) {}, it);
  std::cout << "InvokeFunc\n";
  InvokeFunc(func);

  std::cout << "just pass param, do nothing\n";
  DoNothing(func);
  std::cout << "test functional end\n";

  auto func_cp = func;
  std::cout << "copy func\n";
};

int main(int argc, char** argv) {
  /* TestBatcher(); */
  TestSmartPtr();
  TestPrintPtr();
  TestPrintBool();
  TestAny();
  /* TestExecutable(); */
  TestCurl();
  TestTemp();
  TestFunctional();
  return 0;
}
