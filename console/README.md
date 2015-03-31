### 搭建开发环境
galaxy console是前端和后端分开开发的，如何集成勒，使用工具proxy.go

#### 启动后端服务
```
cd backend && sh localrun.sh
```

#### 启动ui

```
cd ui && grunt serve
```

#### 配置proxy.go

因为前后端是两个服务，所以需要一个reverse proxy
```
go build proxy.go && ./proxy -conf=conf -host=http://0.0.0.0:8234 -port=8080
```

