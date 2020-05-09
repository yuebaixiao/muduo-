// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (giantchen at gmail dot com)

#ifndef MUDUO_BASE_TIMESTAMP_H
#define MUDUO_BASE_TIMESTAMP_H

#include "copyable.h"

#include <stdint.h>
#include <string>

namespace muduo
{

///
/// Time stamp in UTC, in microseconds resolution.
/// Only works for time after the Epoch.
///
/// This class is immutable.
/// It's recommended to pass it by value, since it's passed in register on x64.
///
class Timestamp : public muduo::copyable
               // public boost::less_than_comparable<Timestamp> muduo里此行不是注释
               // public boost::equality_comparable<Timestamp> muduo里此行不是注释
// boost::less_than_comparable作用：只需要自己重载<号运算符，>号运算符会自动生成
// boost::equality_comparable作用：只需要自己重载==号运算符，!=号运算符会自动生成
{
 public:
  ///
  /// Constucts an invalid Timestamp.
  ///
  Timestamp();

  ///
  /// Constucts a Timestamp at specific time
  ///
  /// @param microSecondsSinceEpoch
  explicit Timestamp(int64_t microSecondsSinceEpoch);

  void swap(Timestamp& that)
  {
    std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
  }

  // default copy/assignment/dtor are Okay

  std::string toString() const;
  std::string toFormattedString() const;

  bool valid() const { return microSecondsSinceEpoch_ > 0; }

  // for internal usage.
  int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }

  ///
  /// Get time of now.
  ///
  static Timestamp now();

  ///
  /// Get an invalid time.
  ///
  static Timestamp invalid();

  static const int kMicroSecondsPerSecond = 1000 * 1000; // 1秒 = 1000 * 1000微秒

 private:
  int64_t microSecondsSinceEpoch_; // 微秒
};

inline bool operator<(Timestamp lhs, Timestamp rhs) // Timestamp类的<号运算符重载
{
  return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs) // Timestamp类的==号运算符重载
{
  return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

///
/// Gets time difference of two timestamps, result in seconds.
///
/// @param high, low
/// @return (high-low) in seconds
/// @c double has 52-bit precision, enough for one-microseciond
/// resolution for next 100 years.
inline double timeDifference(Timestamp high, Timestamp low)
{
  int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
  return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond; // 返回2个时间点之间的秒数
}

///
/// Add @c seconds to given timestamp.
///
/// @return timestamp+seconds as Timestamp
///
inline Timestamp addTime(Timestamp timestamp, double seconds)
{
  int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond); // 把秒换算成微秒
  return Timestamp(timestamp.microSecondsSinceEpoch() + delta); // 用微秒生成Timestamp对象
}

}
#endif  // MUDUO_BASE_TIMESTAMP_H
