
all: agent

PROTOBUF_PATH=./thirdparty/protobuf/
PROTOC_PATH=$(PROTOBUF_PATH)/bin/
PBRPC_PATH=./thirdparty/sofa-pbrpc/

PROTOC=$(PROTOC_PATH)protoc

.PHONY: proto

PROTO_FILES = $(wildcard src/proto/*.proto)

proto: $(PROTO_FILES)
	$(PROTOC) --proto_path=./src/proto --proto_path=/usr/local/include \
		      --proto_path=$(PBRPC_PATH)/output/include/ \
		      --proto_path=$(PROTOBUF_PATH)/include \
		      --cpp_out=./src/proto/ $(PROTO_FILES)

agent:
	echo "Make done"

test:
	echo "Test done"
