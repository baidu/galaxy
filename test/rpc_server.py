# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
# Date: 2015-04-29
import socket
import threading
import thread
import struct
import logging as logger

from google.protobuf import service
from sofa.pbrpc import rpc_meta_pb2

class Error(service.RpcException): 
    pass

class TimeoutError(Error): 
    pass

class Connection(object):
    def __init__(self,buffer_size,conn):
        self._buffer_size = buffer_size
        self.conn = conn

    def write(self,data):
        try:
            ret = self.conn.sendall(data)
            if not ret:
                logger.debug("send data successfully")
        except:
            logger.exception("fail to send data")

    def receive(self):
        buffer = self.conn.recv(self._buffer_size)
        if not buffer:
            self.close()
            raise Error("client closed conn")
        return buffer

    def close(self):
        self.conn.close()

    def __enter__(self):
        return self

    def __exit__(self, type, value, tb):
        self.conn.close()

class BlockChannel(object):
    HEADER_SIZE = 24
    MAGIC_STR = 'SOFA'
    def __init__(self,conn):
        self.conn = conn
        self.left_buffer = ""
    def next_block(self):
        buffer  = ""
        if self.left_buffer:
            buffer = self.left_buffer
        buffer += self.conn.receive()
        while len(buffer) < BlockChannel.HEADER_SIZE:
            buffer += self.conn.receive()
        magic_str,meta_size,data_size,message_size = struct.unpack(
        '<4siqq', buffer[0:BlockChannel.HEADER_SIZE])
        if magic_str != BlockChannel.MAGIC_STR:
            raise Error("magic mismatch expect %s but %s"%(BlockChannel.MAGIC_STR,magic_str))
        if message_size != (meta_size+data_size):
            raise Error("invalid header for data size mismatch expect %d but %d"%(message_size,(meta_size+data_size)))
        while len(buffer) < (BlockChannel.HEADER_SIZE+message_size):
            buffer += self.conn.receive()
        meta_end = BlockChannel.HEADER_SIZE+meta_size
        meta_buffer = buffer[BlockChannel.HEADER_SIZE:meta_end]
        data_buffer = buffer[meta_end:meta_end+data_size]
        if len(buffer) > meta_end+data_size:
            self.left_buffer = buffer[meta_end+data_size:]
        return meta_buffer,data_buffer

    def send_block(self,meta_buffer,data_buffer):
        header_bytes = struct.pack(
        '<4siqq', BlockChannel.MAGIC_STR,
        len(meta_buffer), len(data_buffer), len(meta_buffer)+len(data_buffer))
        buffer = header_bytes + meta_buffer + data_buffer
        self.conn.write(buffer)

class Protocol(object):

    def unpack(self,meta_buffer):
        meta = rpc_meta_pb2.RpcMeta()
        meta.ParseFromString(meta_buffer)
        if meta.type != rpc_meta_pb2.RpcMeta.REQUEST:
            raise Error('invalid rpc type except request but respose ')
        if meta.failed:
            return False,meta
        return True,meta

    def pack(self,seq,response,failed=False,reason=""):
        meta = rpc_meta_pb2.RpcMeta()
        meta.type = rpc_meta_pb2.RpcMeta.RESPONSE
        meta.sequence_id = seq
        meta.failed=failed
        meta.reason=reason
        meta_buffer = meta.SerializeToString()
        data_buffer = response.SerializeToString()
        return meta_buffer,data_buffer

class RequestHandler(threading.Thread):
    def __init__(self,conn,service_router):
        threading.Thread.__init__(self)
        self._conn = conn
        self.setDaemon(True)
        self._service_router= service_router
        self._protocol = Protocol()
    def run(self):
        with self._conn:
            block_chan = BlockChannel(self._conn)
            while True:
                meta_buffer,data_buffer = block_chan.next_block()
                status,meta = self._protocol.unpack(meta_buffer)
                if not status:
                    logger.error("fail to unpack buffer for %s"%meta.reason)
                    continue
                full_class,method = self.split_method(meta.method)
                if not full_class:
                    logger.error("class name is None,the method is %s"%meta.method)
                    continue
                service = self._service_router.get(full_class,None)
                if not service:
                    logger.error("no service for class name %s"%full_class)
                    continue
                method_func = service.GetDescriptor().FindMethodByName(method)
                if not method_func:
                    logger.error("no matched method %s in class %s"%(method,full_class))
                    continue
                req_class = service.GetRequestClass(method_func)
                if not req_class:
                    logger.error("no matched req class ")
                    continue
                req = req_class()
                req.ParseFromString(data_buffer)
                response = service.CallMethod(method_func,None ,req, None)
                self.do_response(response,meta.sequence_id,block_chan)

    def do_response(self,response,seq,chan):
        meta_buffer,response_buffer = self._protocol.pack(seq,response)
        chan.send_block(meta_buffer,response_buffer)

    def split_method(self,full_method):
        index = full_method.rindex(".")
        if index < 0:
            return None
        return full_method[:index],full_method[index+1:]
 
class RpcServer(object):
    def __init__(self,port,host="127.0.0.1"):
        self._serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._is_running = False
        self._port = port
        self._host = host
        self._service_router = {}
    def add_service(self,service):
        self._service_router[service.GetDescriptor().full_name] = service

    def start(self):
        self._serversocket.bind((self._host, self._port))
        self._serversocket.listen(5)
        self._is_running = True
        while self._is_running:
            conn, _ = self._serversocket.accept()
            myconn = Connection(4096,conn)
            req_handler = RequestHandler(myconn,self._service_router)
            req_handler.start()

    def stop(self):
        self._is_running = False
        self._serversocket.shutdown(1)

