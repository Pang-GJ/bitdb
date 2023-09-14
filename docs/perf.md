## 记录 perf 分析性能过程
```shell
❯ perf stat ./perf_debug

 Performance counter stats for './perf_debug':

          24408.67 msec task-clock:u              #    0.964 CPUs utilized
                 0      context-switches:u        #    0.000 /sec
                 0      cpu-migrations:u          #    0.000 /sec
            160866      page-faults:u             #    6.591 K/sec
       78056337294      cycles:u                  #    3.198 GHz
         221902531      stalled-cycles-frontend:u #    0.28% frontend cycles idle
         106298847      stalled-cycles-backend:u  #    0.14% backend cycles idle
      155050375182      instructions:u            #    1.99  insn per cycle
                                                  #    0.00  stalled cycles per insn
       35797234846      branches:u                #    1.467 G/sec
          94331995      branch-misses:u           #    0.26% of all branches

      25.330774222 seconds time elapsed

      18.355696000 seconds user
       6.051878000 seconds sys
```
测试结果来看 `task-clock:u` 为 `0.944 CPUs`, 是一个CPU密集型应用？（这与猜测不对啊，难道是IO太少了？)

perf stat还会给出其他几个常用的统计信息：
- task-clock-msecs：CPU利用率，此值越高说明程序的多数时间花费在CPU计算上而非IO；
- context-switches：进程切换次数，记录程序运行过程中发生了多少次进程切换，频繁的进程切换是应该避免的；
- cache-misses：程序运行过程中总体的cache利用情况，如果该值过高，说明程序的cache利用不好；
- CPU-migrations：表示进程t1运行过程中发生了多少次CPU迁移，即被调度器从一个CPU转移到另外一个CPU上运行；
- cycles：处理器时钟，一条指令可能需要多个cycles；
- instructions:机器指令数目；
- IPC：instructions/cycles的比值，该值越大越好，说明程序充分利用了处理器的特性；
- cache-references：cache命中的次数；
- cache-misses：cache失效的次数；

perf record -e cpu-clock -g ./perf_debug
perf report -g
```shell
Samples: 158K of event 'cpu-clock:u', Event count (approx.): 39631500000
  Children      Self  Command     Shared Object         Symbol
+   46.72%     0.00%  perf_debug  perf_debug            [.] main                                                                                                                               ◆
+   46.72%     0.00%  perf_debug  libbitdb.so           [.] bitdb::DB::Open                                                                                                                    ▒
+   24.85%     0.30%  perf_debug  libbitdb.so           [.] bitdb::DB::Merge                                                                                                                   ▒
+   24.29%     0.13%  perf_debug  libbitdb.so           [.] bitdb::index::SkipListIndexer::Put                                                                                                 ▒
+   22.28%     0.38%  perf_debug  libbitdb.so           [.] bitdb::ds::SkipList<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, bitdb::data::LogRecordPst*>::I▒
+   21.30%     0.14%  perf_debug  libbitdb.so           [.] bitdb::DB::LoadIndexFromDataFiles                                                                                                  ▒
+   19.45%     1.29%  perf_debug  libbitdb.so           [.] bitdb::data::DataFile::ReadLogRecord                                                                                               ▒
+   19.39%     2.00%  perf_debug  libbitdb.so           [.] bitdb::ds::SkipList<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, bitdb::data::LogRecordPst*>::F▒
+   18.64%     0.33%  perf_debug  perf_debug            [.] TestPut[abi:cxx11]                                                                                                                 ▒
+   18.14%     0.16%  perf_debug  libbitdb.so           [.] bitdb::DB::Put                                                                                                                     ▒
+   18.04%     0.17%  perf_debug  perf_debug            [.] TestGet                                                                                                                            ▒
+   17.81%     0.18%  perf_debug  libbitdb.so           [.] bitdb::DB::Get                                                                                                                     ▒
+   15.91%    15.91%  perf_debug  libc.so.6             [.] __memcmp_avx2_movbe                                                                                                                ▒
+   15.81%     3.12%  perf_debug  libbitdb.so           [.] std::operator==<char>                                                                                                              ▒
+   15.47%     0.15%  perf_debug  libbitdb.so           [.] bitdb::index::SkipListIndexer::Get                                                                                                 ▒
+   13.79%     0.19%  perf_debug  libbitdb.so           [.] bitdb::ds::SkipList<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, bitdb::data::LogRecordPst*>::F▒
+   12.94%     1.45%  perf_debug  libbitdb.so           [.] bitdb::ds::SkipList<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, bitdb::data::LogRecordPst*>::F▒
+   12.38%     0.11%  perf_debug  libbitdb.so           [.] bitdb::DB::LoadIndexFromDataFiles()::{lambda(bitdb::Bytes const&, bitdb::data::LogRecordType, bitdb::data::LogRecordPst*)#1}::opera▒
+    8.82%     0.23%  perf_debug  libbitdb.so           [.] bitdb::DB::GetValueByLogRecordPst                                                                                                  ▒
+    7.02%     0.49%  perf_debug  libbitdb.so           [.] bitdb::DB::AppendLogRecord                                                                                                         ▒
+    5.97%     1.21%  perf_debug  libbitdb.so           [.] bitdb::data::DataFile::ReadNBytes[abi:cxx11]                                                                                       ▒
+    4.05%     4.05%  perf_debug  libbitdb.so           [.] std::char_traits<char>::compare                                                                                                    ▒
+    3.98%     0.72%  perf_debug  libbitdb.so           [.] std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::basic_string<char*, void>                         ▒
+    3.36%     0.07%  perf_debug  libbitdb.so           [.] bitdb::data::WriteHintRecord                                                                                                       ▒
+    3.31%     0.30%  perf_debug  libbitdb.so           [.] std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::basic_string<__gnu_cxx::__normal_iterator<char*, s▒
+    3.27%     0.33%  perf_debug  libbitdb.so           [.] std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_M_construct<char*>                               ▒
+    3.15%     0.62%  perf_debug  libbitdb.so           [.] bitdb::data::DataFile::Write                                                                                                       ▒
+    3.05%     0.21%  perf_debug  libbitdb.so           [.] std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_M_construct<__gnu_cxx::__normal_iterator<char*, s▒
+    2.98%     0.91%  perf_debug  libbitdb.so           [.] bitdb::data::EncodeLogRecord[abi:cxx11]                                                                                            ▒
+    2.96%     0.36%  perf_debug  libbitdb.so           [.] std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_M_construct_aux<char*>                           ▒
+    2.78%     0.21%  perf_debug  libbitdb.so           [.] std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_M_construct_aux<__gnu_cxx::__normal_iterator<char▒
+    2.76%     2.76%  perf_debug  libstdc++.so.6.0.30   [.] std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::size                                              ▒
+    2.69%     0.56%  perf_debug  libbitdb.so           [.] std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::basic_string<std::allocator<char> >               ▒
+    2.46%     0.07%  perf_debug  libbitdb.so           [.] std::vector<bitdb::ds::SkipList<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, bitdb::data::LogRe▒
+    2.29%     0.32%  perf_debug  libbitdb.so           [.] std::unique_ptr<bitdb::data::DataFile, std::default_delete<bitdb::data::DataFile> >::get                                           ▒
+    2.13%     0.34%  perf_debug  libbitdb.so           [.] std::unique_ptr<bitdb::data::DataFile, std::default_delete<bitdb::data::DataFile> >::operator->                                    ▒
+    2.10%     2.10%  perf_debug  libc.so.6             [.] malloc                                                                                                                             ▒
-    2.06%     0.31%  perf_debug  libbitdb.so           [.] bitdb::data::DecodeLogRecordHeader                                                                                                 ▒
     1.75% bitdb::data::DecodeLogRecordHeader                                                                                                                                                  ▒
+    2.00%     0.17%  perf_debug  libbitdb.so           [.] std::vector<bitdb::ds::SkipList<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, bitdb::data::LogRe▒
+    1.94%     0.51%  perf_debug  libbitdb.so           [.] std::__uniq_ptr_impl<bitdb::data::DataFile, std::default_delete<bitdb::data::DataFile> >::_M_ptr                                   ▒
+    1.86%     1.86%  perf_debug  libstdc++.so.6.0.30   [.] std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::compare                                           ▒
+    1.85%     0.34%  perf_debug  libbitdb.so           [.] std::unique_ptr<bitdb::io::IOHandler, std::default_delete<bitdb::io::IOHandler> >::get                                             ▒
+    1.79%     1.79%  perf_debug  libc.so.6             [.] _int_free                                                                                                                          ▒
Tip: To see callchains in a more compact form: perf report -g folded                                                                                                                          
```

### 火焰图
![flame graph](./flame_graph.svg)