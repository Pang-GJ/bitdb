## Debug 编译
### bitdb
```shell
|               ns/op |                op/s |    err% |          ins/op |          cyc/op |    IPC |         bra/op |   miss% |     total | benchmark
|--------------------:|--------------------:|--------:|----------------:|----------------:|-------:|---------------:|--------:|----------:|:----------
|   41,854,579,913.00 |                0.02 |    0.7% |331,621,888,895.00 |146,115,570,104.00 |  2.270 |74,046,943,403.00 |    0.2% |    460.85 | `db put`
|   26,254,155,305.00 |                0.04 |    0.0% |184,934,127,686.00 |90,903,992,266.00 |  2.034 |41,918,811,336.00 |    0.2% |    289.70 | `db get`

```

### leveldb
```shell
|               ns/op |                op/s |    err% |          ins/op |          cyc/op |    IPC |         bra/op |   miss% |     total | benchmark
|--------------------:|--------------------:|--------:|----------------:|----------------:|-------:|---------------:|--------:|----------:|:----------
|   36,373,268,930.00 |                0.03 |    9.8% |231,951,630,975.00 |86,871,463,668.00 |  2.670 |47,734,581,361.00 |    0.3% |    430.64 | :wavy_dash: `leveldb put` (Unstable with ~1.0 iters. Increase `minEpochIterations` to e.g. 10)
|   13,273,592,727.00 |                0.08 |    0.3% |164,216,715,004.00 |58,015,308,186.00 |  2.831 |31,717,481,980.00 |    0.1% |    146.09 | `leveldb get`

```

## Release 编译
开启编译参数 `-Ofast` 。

### bitdb
```shell
❯ ./benchmark

|               ns/op |                op/s |    err% |          ins/op |          cyc/op |    IPC |         bra/op |   miss% |     total | benchmark
|--------------------:|--------------------:|--------:|----------------:|----------------:|-------:|---------------:|--------:|----------:|:----------
|   26,961,535,057.00 |                0.04 |    0.4% |220,138,595,050.00 |84,195,601,238.00 |  2.615 |48,931,670,623.00 |    0.3% |    297.35 | `db put`
|   13,380,437,374.00 |                0.07 |    0.0% |81,581,796,204.00 |35,145,600,930.00 |  2.321 |17,885,850,417.00 |    0.3% |    147.20 | `db get`
```

### leveldb
链接的 leveldb 可能没开 `-Ofast`，后面需要补测一下只有 `-O2` 的测试。
```shell
❯ ./benchmark_leveldb

|               ns/op |                op/s |    err% |          ins/op |          cyc/op |    IPC |         bra/op |   miss% |     total | benchmark
|--------------------:|--------------------:|--------:|----------------:|----------------:|-------:|---------------:|--------:|----------:|:----------
|   32,400,586,009.00 |                0.03 |    6.1% |231,924,378,739.00 |86,703,791,124.00 |  2.675 |47,725,453,340.00 |    0.3% |    368.14 | :wavy_dash: `leveldb put` (Unstable with ~1.0 iters. Increase `minEpochIterations` to e.g. 10)
|   12,692,311,720.00 |                0.08 |    0.2% |158,019,621,547.00 |55,963,984,933.00 |  2.824 |30,491,028,564.00 |    0.1% |    139.91 | `leveldb get`
```

## 内存索引数据结构
```shell
❯ ./bench_index_ds

|               ns/op |                op/s |    err% |          ins/op |          cyc/op |    IPC |         bra/op |   miss% |     total | benchmark
|--------------------:|--------------------:|--------:|----------------:|----------------:|-------:|---------------:|--------:|----------:|:----------
|      110,246,894.50 |                9.07 |    0.2% |1,869,999,871.00 |  484,416,307.00 |  3.860 | 420,000,009.00 |    0.0% |      1.27 | `hashmap put`
|       17,735,010.00 |               56.39 |    0.4% |  199,999,995.00 |   77,826,962.00 |  2.570 |  40,000,003.00 |    0.0% |      0.18 | `hashmap get`
|       18,749,358.50 |               53.34 |    0.4% |  189,999,996.00 |   82,470,982.50 |  2.304 |  40,000,003.00 |    0.0% |      0.19 | `hashmap get reverse`
|    1,065,697,372.50 |                0.94 |    0.5% |2,518,715,217.50 |4,684,736,308.50 |  0.538 | 579,034,093.00 |    0.1% |     10.63 | `hashmap put random`
|      607,784,108.50 |                1.65 |    0.5% |  838,724,915.50 |2,679,451,535.50 |  0.313 | 199,036,413.00 |    0.2% |      6.10 | `hashmap get random`
|      862,905,083.00 |                1.16 |    0.2% |3,812,744,107.50 |3,805,823,048.00 |  1.002 | 698,155,839.50 |    0.5% |      9.29 | `treemap put`
|      783,029,348.50 |                1.28 |    0.2% |2,060,244,158.50 |3,451,064,867.50 |  0.597 | 648,224,613.50 |    5.0% |      7.83 | `treemap get`
|      890,358,305.00 |                1.12 |    0.1% |2,050,244,216.50 |3,919,964,189.50 |  0.523 | 648,224,670.50 |    5.0% |      8.90 | `treemap get reverse`
|    7,750,157,058.50 |                0.13 |    0.2% |4,369,019,763.00 |34,125,105,792.50 |  0.128 | 846,266,841.00 |    1.8% |     77.53 | `treemap put random`
|    7,423,420,554.50 |                0.13 |    0.2% |2,608,214,996.50 |32,688,828,654.50 |  0.080 | 775,452,803.00 |   15.3% |     74.28 | `treemap get random`
|      827,240,709.00 |                1.21 |    0.3% |6,544,680,282.50 |3,641,439,157.00 |  1.797 |2,110,468,894.50 |    1.0% |      8.73 | `skiplist put`
|      617,818,799.00 |                1.62 |    0.1% |4,188,165,877.50 |2,724,600,103.00 |  1.537 |1,563,874,283.50 |    1.4% |      6.18 | `skiplist get`
|      650,795,563.00 |                1.54 |    0.1% |4,178,165,881.50 |2,870,356,210.50 |  1.456 |1,563,874,286.50 |    1.0% |      6.51 | `skiplist get reverse`
|    7,230,386,261.50 |                0.14 |    0.1% |6,995,194,764.00 |31,827,468,446.50 |  0.220 |2,183,967,164.50 |    4.3% |     72.34 | `skiplist put random`
|    7,000,059,108.50 |                0.14 |    0.3% |4,588,557,372.50 |30,814,314,179.50 |  0.149 |1,637,309,800.00 |    5.7% |     69.93 | `skiplist get random`

```