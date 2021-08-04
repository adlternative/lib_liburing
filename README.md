# lib_lib_uring
一个用来自己玩耍 c++ 17/20 新特性和 linux 内核 io_uring 异步 io 新特性的封装库。

## 构建
```
mkdir build
cd build
cmake ..
make
```

### 依赖

* `gflags`
* `spdlog`
* `fmt`
* `gtest`

### 示例
* `uring_cp`: 拷贝文件。

```bash
cd build
./uring_cp ../resource/tux.png ../resource/tux-fun.png
diff ../resource/tux.png ../resource/tux-fun.png
```

是的，拷贝成功了。

你还可以通过命令行参数 `--queue_depth=<depth>` 以及 `--block_size=<size>` 调整每个请求的块大小，后期会添加更加丰富的参数。

```bash
cd build
./uring_cp ../resource/tux.png ../resource/tux-fun.png --queue_depth=32 --block_size=1024
```

### 贡献
欢迎进行贡献和指正。