#include <iostream>
#include <sstream>
#include <cstdlib>

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

#include "thing_container.h"

class TestA {};
class TestB : public TestA {};

void TestSmartPtr() {
  ThingContainer ic;

  auto m = std::shared_ptr<void>(malloc(400), [](void* ptr) {free(ptr);});
  char* a = static_cast<char*>(m.get());
  snprintf(a, 100, "test shared_ptr<void>");
  std::cout << a << std::endl;

  std::shared_ptr<TestB> b_p = std::make_shared<TestB>();
  std::shared_ptr<TestA> a_p = b_p;
  std::cout << "derived sptr == base sptr? : " << std::boolalpha << (a_p == b_p) << std::endl;
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

  std::cout << "################copy any\n";
  any to1;
  to1 = aitem;
  std::cout << "################end copy any\n";

  std::cout << "---------------\n";
  auto& lref = any_cast<OneItem&>(aitem);
  std::cout << "---------------\n";
  std::cout << "################move any\n";
  any to2;
  to2 = std::move(aitem);
  std::cout << "################end move any\n";
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

/*
template <typename... Args>
class ClassWithArgs {
 public:
  using ProcessFunc = std::function<void(Args...)>;
  ClassWithArgs(const ProcessFunc& func) : func_(func) {}
  void operator()(Args&&... args) {
    func_(std::forward<Args>(args)...);
  };

 private:
  ProcessFunc func_;
};

void EmptyFunction(int a, const std::string& str) {
  std::cout << a << ": " << str << "\n";
}

void TestEmit(int a, const std::string& str) {
  ClassWithArgs<int, const std::string&> temp_class(&EmptyFunction);
  temp_class(a, str);
}
*/

#include "temp.h"
void TestTemp() {
  aContainer a;
  int i_a;
  a.PrintTypeAddress(i_a);
  PrintIntTypeAddress();

  /* TestEmit(2, "some string"); */
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

#include <queue>
void TestPriorityQueue() {
  std::priority_queue<OneItem, std::deque<OneItem>> q;
  q.emplace(0, 0);
  std::cout << "@@@@@@@@@@@@@@@@@@@@ push 0 0\n";
  q.emplace(1, 1);
  std::cout << "@@@@@@@@@@@@@@@@@@@@ push 1 1\n";
  q.emplace(0, 2);
  std::cout << "@@@@@@@@@@@@@@@@@@@@ push 0 2\n";
  q.emplace(0, 3);
  std::cout << "@@@@@@@@@@@@@@@@@@@@ push 0 3\n";
  q.emplace(1, 4);
  std::cout << "@@@@@@@@@@@@@@@@@@@@ push 1 4\n";
  q.emplace(2, 5);
  std::cout << "@@@@@@@@@@@@@@@@@@@@ push 2 5\n";
  q.emplace(1, 6);
  std::cout << "@@@@@@@@@@@@@@@@@@@@ push 1 6\n";

  std::cout << "@@@@@@@@@@@@@@@@@@@@ test priority queue\n";
  while (!q.empty()) {
    auto item = q.top();
    std::cout << item.priority << "\t" << item.sequence << "\n";
    q.pop();
  }
  std::cout << "@@@@@@@@@@@@@@@@@@@@ end\n";
}

void DoThrow() {
  throw std::runtime_error("some exception");
}

void TestNoexcept() noexcept {
  try {
    DoThrow();
  } catch(...) {
  }
}

#include <map>
void TestMap() {
  std::map<std::string, std::string> m;
  m["test1"] = "test 1";
  m["test2"] = "test 2";
  try {
    std::cout << m.at("test3") << "\n";
  } catch (std::exception& e) {
    std::cerr << e.what() << "\n";
  }
}

void TestPrintf() {
  printf("%-16s END %-16s\n", "test", "");
};

class NoMove {
 public:
  NoMove() = default;
  NoMove(const NoMove&) = default;
  NoMove& operator=(const NoMove&) = default;
  NoMove(NoMove&&) = delete;
  NoMove& operator=(NoMove&&) = delete;
};

void TestMoveAny() {
  NoMove a;
  any contain = a;
  std::cout << std::boolalpha << std::is_move_constructible<decltype(contain)>::value << std::endl;
  /* any another = std::move(contain); */
}

void RrefParamFunc(OneItem&& item) {
}

void TestRrefParam() {
  OneItem item;
  // compile error
  /* RrefParamFunc(item); */
  RrefParamFunc(OneItem());
}

void TestBit() {
  std::cout << "test bit\n";
  int num_1 = 1023;
  std::cout << (num_1 & 1) << std::endl;
  std::cout << (num_1 | 1024) << std::endl;
  std::cout << (num_1 | 1023) << std::endl;
  std::cout << (num_1 & num_1 + 1) << std::endl;

  int64_t e = -5;
  int64_t i1 = int64_t(1) << 56;
  int64_t i2 = int64_t(2) << 56;
  std::cout << i1 + e << std::endl;
  std::cout << 2 * (i1 + e) << std::endl;
  std::cout << i2 + e << std::endl;
  std::cout << "test bit end\n";
}

#ifndef __linux__
#include "windows.h"
#else

#include "unistd.h"
#include "sys/sysinfo.h"
#endif

void TestGetProcessorNumber() {
#ifndef __linux__
SYSTEM_INFO sysInfo;
GetSystemInfo( &sysInfo );
printf("system cpu num is %d\n", sysInfo.dwNumberOfProcessors);
#else

printf("system cpu num is %d \n", sysconf(_SC_NPROCESSORS_CONF));
printf("system enable num is %d\n", sysconf(_SC_NPROCESSORS_ONLN));

//GNU way
printf("system cpu num is %d\n", get_nprocs_conf());
printf("system enable num is %d\n", get_nprocs());
#endif
}

/*
template <typename... Args>
void ArgsFunc(Args&&... args, int major, int64_t minor = 0) {
}

void TestArgsFunc() {
  ArgsFunc("seg", 124, 2351, 123);
  ArgsFunc("seg", "sdf", 2351, 123, 125);
}
*/

#include <limits>
void TestBigNumber() {
  std::cout << std::numeric_limits<uint32_t>::max() - 1 << std::endl;
  std::cout << std::numeric_limits<uint32_t>::max() + 1 << std::endl;
  std::cout << std::numeric_limits<uint32_t>::min() - 1 << std::endl;
}

void TestQueue() {
  std::queue<int> q;
  // segment fault
  /* std::cout << q.back() << "\n"; */
}

void SlotFunc(int a, const std::string& b) {
}

#include "sigslot.h"
class SignalClass {
 public:
  void EmitSignal(int a, const std::string& b) {
    emit sig_a(a, b);
  }
 signals:
  Signal<SignalPolicy::SYNC, int, const std::string&> sig_a;
};

void TestSigSlot() {
  SignalClass sig_c;
  connect(&sig_c, sig_a, SlotFunc);
  sig_c.EmitSignal(5, "test signal slot");
}

class DestructorCatch {
 public:
  ~DestructorCatch() {
    try {
    } catch(...) {
      std::cout << "catch exception\n";
    }
  }
};

void TestCatch() {
  DestructorCatch c;
  throw std::runtime_error("some error");
}

void TestVector() {
  std::cout << "test vector" << std::endl;
  float* a = new float[10];
  a[0] = 0;
  a[1] = 1;
  a[2] = 2;
  std::vector<float> v_a(a, a + 9);
  a[0] = 0;
  a[1] = 0;
  a[2] = 0;
  for (auto& it : v_a) {
    std::cout << it << ", ";
  }
  std::cout << std::endl;
  delete[] a;
  for (auto& it : v_a) {
    std::cout << it << ", ";
  }
  std::cout << std::endl;

  v_a.resize(0);
}

void TestAtomic() {
  std::vector<std::thread> ths;
  std::atomic<bool> flag(false);
  /* flag.store(false, std::memory_order_release); */
  int thread_num = 10;
  ths.reserve(thread_num);
  for (int i = 0; i < thread_num; ++i) {
    ths.emplace_back([&flag](){
      int time = 100;
      int count = 0;
      bool f;
      while (time--) {
        f = flag.load(std::memory_order_acquire);
        if (!f) {
          f = true;
        }
        std::cout << "a" << "b" << "c" << std::endl;
        if (count++ == 8) {
          count = 0;
          f = false;
        }
        flag.store(f, std::memory_order_release);
      }
    });
  }
  for (size_t i = 0; i < ths.size(); ++i) {
    ths[i].join();
  }
}

struct AnyContainer {
  template <typename T>
  void Set(T&& v) {
    a = std::forward<T>(v);
  }

  template <typename T>
  typename std::remove_reference<T>::type Get() const {
    return any_cast<typename std::remove_reference<T>::type>(a);
  }

  template <typename T>
  typename std::add_lvalue_reference<T>::type GetLref() & {
    return any_cast<typename std::add_lvalue_reference<T>::type>(a);
  }

  any a;
};

void TestAnyContainer() {
  AnyContainer c;
  /* std::string str("test"); */
  /* c.Set(str); */
  c.Set(OneItem());
  std::cout << "@@@@@@@@@@@\n";
  auto& a = c.GetLref<OneItem>();
  std::cout << "@@@@@@@@@@@\n";

  /* const auto& cc = c; */
  /* std::string s = cc.Get<std::string>(); */
  /* s = cc.Get<std::string&>(); */
  /* s.erase(0); */
  /* std::cout << cc.Get<std::string>() << std::endl; */
  /* s = cc.Get<std::string&&>(); */
}

template <int T>
struct TraitsBase {
  static constexpr const char* type = "unknown";
};

template <>
struct TraitsBase<1> {
  static constexpr const char* type = "int";
};

template <>
struct TraitsBase<2> {
  static constexpr const char* type = "float";
};

constexpr const char* GetTraitsName(const int i) {
  return i == 1 ? TraitsBase<1>::type : (i == 2 ? TraitsBase<2>::type : TraitsBase<0>::type);
}

void TestConstexpr() {
  int a = 1;
  std::cout << GetTraitsName(a) << std::endl;
}

inline void RealFunc(OneItem item) {
}

inline void InlineFunc(OneItem&& item) {
  return RealFunc(std::forward<OneItem>(item));
  /* std::cout << item.data << std::endl; */
}

class TestInlineReturn {
 public:
  OneItem GetItem() { return item_; }

 private:
  OneItem item_;
};

void TestInline() {
  OneItem item;
  item.data = "test inline";
  std::cout << "test inline begin\n";
  InlineFunc(std::move(item));
  std::cout << "test inline end\n";

  TestInlineReturn t;
  std::cout << "test inline return\n";
  std::cout << t.GetItem() << std::endl;
  std::cout << t.GetItem().data << std::endl;
  std::cout << "test inline return end\n";
}

void TestFuture() {
  std::promise<int> p;
  auto f = p.get_future();
  std::cout << "before move] future valid: " << f.valid() << std::endl;
  auto f_m = std::move(f);
  std::cout << "after move] future valid: " << f.valid() << std::endl;
  std::cout << "after move] new future valid: " << f_m.valid() << std::endl;
  p.set_value(1);
  std::cout << "after set value] new future valid: " << f_m.valid() << std::endl;
  f_m.get();
  std::cout << "after get] new future valid: " << f_m.valid() << std::endl;
}

class CrtpBase {
 public:
  explicit CrtpBase(const std::string& name) : name_(name) {}
  virtual void PrintName() = 0;
  const std::string& Name() {
    return name_;
  }
 private:
  std::string name_;
};

template <typename T>
class Crtp : public CrtpBase {
 public:
  explicit Crtp(const std::string& name) : CrtpBase(name) {}
  void PrintName() {
    std::cout << static_cast<T*>(this)->GetSpecialName() << std::endl;
  }
};

class DerivedCrtp : public Crtp<DerivedCrtp> {
 public:
  explicit DerivedCrtp(const std::string& name) : Crtp(name) {}
  const std::string& GetSpecialName() {
    return Name();
  }
};

void TestCrtp() {
  DerivedCrtp instance("derived");
  CrtpBase* p = &instance;
  p->PrintName();
}

#include "unistd.h"
#define PATH_MAX_LENGTH 1024
void TestGetPath() {
  char path[PATH_MAX_LENGTH];
  int cnt = readlink("/proc/self/exe", path, PATH_MAX_LENGTH);
  if (cnt < 0 || cnt >= PATH_MAX_LENGTH) {
    std::cout << "read link error!!!!!";
  }
  path[cnt] = '$';
  std::cout << "origin: " << path << std::endl;
  path[cnt] = '\0';
  std::string result(path);
  std::string subfix = "build/";
  std::string res = result.substr(0, result.find_last_of('/') + 1);
  std::string res_tail = res.substr(res.size() - subfix.size(), res.size());
  std::cout << "subfix: " << subfix << std::endl;
  std::cout << "res: " << res << std::endl;
  std::cout << "res_tail: " << res_tail << std::endl;
  if (res_tail == subfix) {
    std::cout << "success\n";
    /* res = res.substr(0, res.size() - subfix.size()); */
  }
}

#define OFFSET(Type, member) (size_t)&(((Type*)0)->member)

struct TestSize{
  std::function<void(int, OneItem)> func;
  OneItem item;
  int64_t i64;
  uint32_t u32;
  std::atomic<int> number{0};
  std::atomic<bool> flag_a{false};
  std::atomic<bool> flag_b{false};
};

struct BBox {
  float x;
  float y;
  float w;
  float h;
};

struct Object1 {
  int label;
  float score;
  BBox box;
  int detect_id;
  int track_id;
  std::vector<float> feature;
};

struct Object2 {
  BBox box;
  int label;
  float score;
  int detect_id;
  int track_id;
  std::vector<float> feature;
};

void TestSizeOf() {
  std::atomic<bool> a_b;
  std::cout << "size of atomic<bool>: " << sizeof(a_b) << std::endl;
  std::cout << "size of TestSize: " << sizeof(TestSize) << std::endl;
  std::cout << "  size of function: " << sizeof(std::function<void(int, OneItem)>) << std::endl;
  std::cout << "  size of item: " << sizeof(OneItem) << std::endl;
  std::cout << "  size of number: " << sizeof(std::atomic<int>) << std::endl;
  std::cout << "offset: " << OFFSET(TestSize, func)
            << " " << OFFSET(TestSize, item)
            << " " << OFFSET(TestSize, i64)
            << " " << OFFSET(TestSize, u32)
            << " " << OFFSET(TestSize, number)
            << " " << OFFSET(TestSize, flag_a)
            << " " << OFFSET(TestSize, flag_b)
            << std::endl;
  std::cout << "offset: " << offsetof(TestSize, func)
            << " " << offsetof(TestSize, item)
            << " " << offsetof(TestSize, i64)
            << " " << offsetof(TestSize, u32)
            << " " << offsetof(TestSize, number)
            << " " << offsetof(TestSize, flag_a)
            << " " << offsetof(TestSize, flag_b)
            << std::endl;
  TestSize a;
  a.flag_a.store(true);
  std::cout << "flag_a: " << a.flag_a.load() << std::endl;
  a.flag_b.store(true);
  std::cout << "flag_a: " << a.flag_a.load() << std::endl;

  std::cout << "sizeof vector<float>: " << sizeof(std::vector<float>) << std::endl;
  std::cout << "sizeof Object1: " << sizeof(Object1) << std::endl;
  std::cout << "sizeof Object2: " << sizeof(Object2) << std::endl;
}

void TestMemFn() {
  OneItem item;
  auto access = std::mem_fn(&OneItem::data);
  std::cout << access(&item) << std::endl;
}

void PopFrom(std::priority_queue<OneItem>* q, OneItem* value) {
  *value = std::move(const_cast<OneItem&>(q->top()));
}

void TestMoveLref() {
  std::priority_queue<OneItem> q;
  q.emplace();
  OneItem item;
  OneItem& item_ref = item;
  std::cout << "before pop" << std::endl;
  PopFrom(&q, &item_ref);
  std::cout << "after pop" << std::endl;
}
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
  TestPriorityQueue();
  TestNoexcept();
  TestMap();
  TestPrintf();
  TestMoveAny();
  TestRrefParam();
  TestBit();
  TestGetProcessorNumber();
  /* TestArgsFunc(); */
  TestBigNumber();
  TestQueue();
  TestSigSlot();
  /* TestCatch(); */
  TestVector();
  /* TestAtomic(); */
  TestAnyContainer();
  TestConstexpr();
  TestInline();
  TestFuture();
  TestCrtp();
  TestGetPath();
  TestSizeOf();
  TestMemFn();
  TestMoveLref();
  return 0;
}
