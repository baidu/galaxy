// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "unit_test.h"

#ifdef TEST_COLLECTOR_ENGINE_ON
#include "agent/collector/collector.h"
#include "agent/collector/collector_engine.h"
#include "agent/util/error_code.h"

class TestCollectorEngine : public testing::Test {
protected:
    static void SetUpTestCase() {
    }

    static void TearDownTestCase() {
    }
};

class CollectorForTest : public baidu::galaxy::collector::Collector {
public:
    CollectorForTest() {}
    ~CollectorForTest() {}
    virtual baidu::galaxy::util::ErrorCode Collect() {
        std::cerr << "Collect ok .." << std::endl;
        return ERRORCODE_OK;
    }

    virtual void Enable(bool enabled) {
        enable_ = enabled;
    }

    virtual bool Enabled() {
        return enable_;
    }

    virtual bool Equal(const Collector& r) {
        return this->Name() == r.Name();
    }

    virtual int Cycle() {
        return 5;
    }

    virtual std::string Name() const {
        return "collector_for_test";
    }

private:
    bool enable_;
};

TEST_F(TestCollectorEngine, Register_Unregister)
{
    boost::shared_ptr<baidu::galaxy::collector::CollectorEngine> ce
        = baidu::galaxy::collector::CollectorEngine::GetInstance();

    boost::shared_ptr<CollectorForTest> collector(new CollectorForTest());
    baidu::galaxy::util::ErrorCode ec = ce->Register(collector);
    EXPECT_EQ(0, ec.Code());
    ec = ce->Register(collector);
    EXPECT_EQ(-1, ec.Code()) << ec.Message();
    ce->Unregister(collector);
    ec = ce->Register(collector);
    EXPECT_EQ(0, ec.Code());
    ce->Unregister(collector);
}

TEST_F(TestCollectorEngine, Setup)
{
    boost::shared_ptr<baidu::galaxy::collector::CollectorEngine> ce
        = baidu::galaxy::collector::CollectorEngine::GetInstance();

    boost::shared_ptr<CollectorForTest> collector(new CollectorForTest());
    collector->Enable(true);
    baidu::galaxy::util::ErrorCode ec = ce->Register(collector);
    EXPECT_EQ(0, ec.Code());
    ce->Setup();
    sleep(10);
    ce->TearDown();
}

#endif
