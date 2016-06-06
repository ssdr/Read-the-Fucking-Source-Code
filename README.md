# Read the Fucking Source Code

该项目fork自[reading-code-of-nginx-1.9.2](https://github.com/y123456yz/reading-code-of-nginx-1.9.2)，但由于原项目结构较混乱，这对于有clean强迫症的人来说真是不能忍 :broken_heart:
另外，原项目作者是在Windows下的Source Insight做的代码注释，这在我们这些Vim党环境下是乱码的 :broken_heart:
所以，笔者打算对该项目做重新梳理，当然最主要的目的还是阅读开源代码 :smiley:

## 关于乱码问题

如果在你的环境中遇到注释乱码的情况，请执行以下命令：

> file src/* | grep ISO-8859 | awk -F':' '{print $1}' | xargs -I {} ./iconv.sh {}

其中，iconv.sh在notes目录下。
