### 知识点梳理 ###
本部分介绍TcpServer，并初步实现TcpConnection，本节只处理连接的建立，下一节处理连接的断开，再往后以此处理读取数据和发送数据。

TcpServer的功能是管理accept(2)获得的TcpConnection.TcpServer是供用户使用的，生命周期由用户控制。用户只需要设置好callback，再调用start即可。

每个TcpConnection对象有一个名字，这个名字是由其所属的TcpServer在创建TcpConnection对象时生成，名字是ConnectionMap的key。

在新连接到达时，Acceptor会回调newConnection()，后者会创建TcpConnection对象conn，把它加入ConnectionMap，设置好callback，再调用conn->connectEstablished()，其中会回调用户提供的ConnectionCallback。

TcpConnection是muduo里唯一默认使用的shared_ptr来管理的class，也是唯一继承std::enable_shared_from_this的class。使用用它的原因：当执行用户提供的callback时，必须保证TcpConnection对象是存活的，std::enable_shared_from_this保证了它是存活的。

- 理解std::enable_shared_from_this的例子

下面的例子是错误的，同一个对象的析构函数执行了2次。
``` c++
#include <memory>
#include <iostream>

class Bad
{
public:
        std::shared_ptr<Bad> getptr() {
                return std::shared_ptr<Bad>(this);
        }
        ~Bad() { std::cout << "Bad::~Bad() called" << std::endl; }
};

int main()
{
        // 错误的示例，每个shared_ptr都认为自己是对象仅有的所有者
        std::shared_ptr<Bad> bp1(new Bad());
        std::shared_ptr<Bad> bp2 = bp1->getptr();
        // 打印bp1和bp2的引用计数
        std::cout << "bp1.use_count() = " << bp1.use_count() << std::endl;
        std::cout << "bp2.use_count() = " << bp2.use_count() << std::endl;
}

```

使用std::enable_shared_from_this解决保活问题

``` c++
#include <memory>
#include <iostream>

struct Good : std::enable_shared_from_this<Good> // 注意：继承
{
public:
        std::shared_ptr<Good> getptr() {
                return shared_from_this();
        }
        ~Good() { std::cout << "Good::~Good() called" << std::endl; }
};

int main()
{
        // 大括号用于限制作用域，这样智能指针就能在system("pause")之前析构
        {
                std::shared_ptr<Good> gp1(new Good());
                std::shared_ptr<Good> gp2 = gp1->getptr();
                // 打印gp1和gp2的引用计数
                std::cout << "gp1.use_count() = " << gp1.use_count() << std::endl;
                std::cout << "gp2.use_count() = " << gp2.use_count() << std::endl;
        }
}

```
