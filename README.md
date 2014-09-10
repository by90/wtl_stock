stock
=====

tutorials for c++ 11 to develop real world application,using wtl for windows gui.

now I've implemented such futures:  
  1. a wtl simple sdi navigate framework,using toolbar switch between modeless dialog.  
  2. mvvm framework for wtl modeless dialog.  
  3. multi thread progress report,with simple lambda function,so we can test business logic,don't need pay attention to ui elements.  
  4. try to make a paser for binary files or memory block,using simple input iterator.  
  5. import quote from a binary file.  
  6. import exright files from a binary file,under same  import quote dialog,so we can switch models form view model.  
  7. a sqlite c++ 11 wrapper,which is the best wrapper now,you could find some info in dbtest.cc  
  8. comiled under vs2013,and using sqlite.  

    这是一个简单的示例，用于导入其他证券软件的行情数据、财务资料和除权除息数据，实现复权算法。这个项目的目的，是熟悉c++ 11、使用wtl  
    编写windows客户端应用。  
    虽然简单，但完全按照真实项目开发过程，做仔细的需求分析、用户体验分析、并采用界面驱动设计的方式详尽设计，力图完善每一个细节。  
    本项目的成果，是一个提供全部代码的证券分析软件的基础，我想，解决了数据接口、提供了基本的界面之后，要编写自己的证券、期货、外汇  
    分析软件、或实现自己的分析算法，门槛将大为降低。  
    很欢迎同好加入，事实上，继续做下去，还有很多事情可干：  
    1、实时行情接口  
    2、K线图和技术图表  
    3、行情列表  
    4、键盘精灵  
    5、指标体系  
    ...  
    这些都是简单的基础，但工作量不小，若有熟悉c++的同行有兴趣加入，或在运行过程中发现Bug，请在项目的issue列表里提出。

