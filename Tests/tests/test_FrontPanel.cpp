#include <gtest/gtest.h>

#include "FrontPanel.h"

#include "FactoriesImplementation.h"

#include "FrontPanelMock.h"
#include "FrontPanelTextDisplayMock.h"
#include "FrontPanelConfigMock.h"
#include "IarmBusMock.h"
#include "ServiceMock.h"

#include "dsMgr.h"

using namespace WPEFramework;

class FrontPanelTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::FrontPanel> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    FrontPanelTest()
        : plugin(Core::ProxyType<Plugin::FrontPanel>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
    }
    virtual ~FrontPanelTest() = default;
};

class FrontPanelDsTest : public FrontPanelTest {
protected:
    FrontPanelIndicatorImplMock frontPanelIndicatorImplMock;
    FrontPanelIndicatorImplMock frontPanelIndicatorImplStringMock;
    FrontPanelIndicatorImplMock frontPanelIndicatorImplIntMock;
    FrontPanelTextDisplayImplMock frontPanelTextDisplayImplStringMock;
    FrontPanelTextDisplayImplMock frontPanelTextDisplayImplMock;
    FrontPanelTextDisplayImplMock frontPanelTextDisplayImplIntMock;
    FrontPanelConfigImplMock frontPanelConfigImplMock;
    FrontPanelDsTest()
        : FrontPanelTest()
    {
        device::FrontPanelIndicator::getInstance().impl = &frontPanelIndicatorImplMock;
	device::FrontPanelIndicator::getInstance("Power").impl = &frontPanelIndicatorImplStringMock;
        device::FrontPanelIndicator::getInstance(0).impl = &frontPanelIndicatorImplIntMock;
	device::FrontPanelTextDisplay::getInstance("Text").impl = &frontPanelTextDisplayImplStringMock;
	device::FrontPanelTextDisplay::getInstance(0).impl = &frontPanelTextDisplayImplIntMock;
	device::FrontPanelConfig::getInstance().impl = &frontPanelConfigImplMock;
        device::FrontPanelTextDisplay::getInstance().impl = &frontPanelTextDisplayImplMock;

    }
    virtual ~FrontPanelDsTest() override
    {
        device::FrontPanelIndicator::getInstance().impl = nullptr;
	device::FrontPanelIndicator::getInstance("Power").impl = nullptr;
	device::FrontPanelTextDisplay::getInstance("Text").impl = nullptr;
	device::FrontPanelConfig::getInstance().impl = nullptr;
        device::FrontPanelTextDisplay::getInstance().impl = nullptr;
	device::FrontPanelTextDisplay::getInstance(0).impl = nullptr;
        device::FrontPanelIndicator::getInstance(0).impl = nullptr;

    }
};

class FrontPanelInitializedTest : public FrontPanelTest {
protected:
    IarmBusImplMock iarmBusImplMock;
    IARM_EventHandler_t dsPanelEventHandler;
    FrontPanelConfigImplMock frontPanelConfigImplMock;
    FrontPanelIndicatorImplMock frontPanelIndicatorImplMock;
    IARM_EventHandler_t dsFrontPanelModeChange;


    FrontPanelInitializedTest()
        : FrontPanelTest()
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;
        device::FrontPanelConfig::getInstance().impl = &frontPanelConfigImplMock;
        device::FrontPanelIndicator::getInstance().impl = &frontPanelIndicatorImplMock;

        ON_CALL(iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                    if ((string(IARM_BUS_PWRMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_PWRMGR_EVENT_MODECHANGED)) {
                        EXPECT_TRUE(handler != nullptr);
                        dsFrontPanelModeChange = handler;
                    }

                    return IARM_RESULT_SUCCESS;
                }));
	FrontPanelIndicatorImplMock frontPanelIndicatorMock;

    device::FrontPanelIndicator indicatorList;

        indicatorList.impl = &frontPanelIndicatorMock;
    ON_CALL(frontPanelConfigImplMock, getIndicators())
        .WillByDefault(::testing::Return(device::List<device::FrontPanelIndicator>({indicatorList})));
    ON_CALL(frontPanelIndicatorImplMock, getName())
        .WillByDefault(::testing::Return("red"));
        EXPECT_EQ(string(""), plugin->Initialize(nullptr));
    }
    virtual ~FrontPanelInitializedTest() override
    {
        device::FrontPanelConfig::getInstance().impl = nullptr;
        device::FrontPanelIndicator::getInstance().impl = nullptr;

	plugin->Deinitialize(nullptr);
   
        IarmBus::getInstance().impl = nullptr;
    }
};

class FrontPanelInitializedEventTest : public FrontPanelInitializedTest {
protected:
    ServiceMock service;
    Core::JSONRPC::Message message;
    FactoriesImplementation factoriesImplementation;
    PluginHost::IDispatcher* dispatcher;

    FrontPanelInitializedEventTest()
        : FrontPanelInitializedTest()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);

        dispatcher = static_cast<PluginHost::IDispatcher*>(
            plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service);
    }

    virtual ~FrontPanelInitializedEventTest() override
    {
        dispatcher->Deactivate();
        dispatcher->Release();

        PluginHost::IFactories::Assign(nullptr);
    }
};

class FrontPanelInitializedEventDsTest : public FrontPanelInitializedEventTest {
protected:
    FrontPanelIndicatorImplMock frontPanelIndicatorImplStringMock;
    FrontPanelIndicatorImplMock frontPanelIndicatorImplIntMock;
    FrontPanelTextDisplayImplMock frontPanelTextDisplayImplStringMock;
    FrontPanelTextDisplayImplMock frontPanelTextDisplayImplMock;
    FrontPanelTextDisplayImplMock frontPanelTextDisplayImplIntMock;


    FrontPanelInitializedEventDsTest()
        : FrontPanelInitializedEventTest()
    {
        device::FrontPanelIndicator::getInstance("Power").impl = &frontPanelIndicatorImplStringMock;
        device::FrontPanelIndicator::getInstance(0).impl = &frontPanelIndicatorImplIntMock;
        device::FrontPanelTextDisplay::getInstance("Text").impl = &frontPanelTextDisplayImplStringMock;
        device::FrontPanelTextDisplay::getInstance(0).impl = &frontPanelTextDisplayImplIntMock;
        device::FrontPanelTextDisplay::getInstance().impl = &frontPanelTextDisplayImplMock;
	
    }

    virtual ~FrontPanelInitializedEventDsTest() override
    {
        device::FrontPanelIndicator::getInstance("Power").impl = nullptr;
        device::FrontPanelTextDisplay::getInstance("Text").impl = nullptr;
        device::FrontPanelTextDisplay::getInstance().impl = nullptr;
	device::FrontPanelTextDisplay::getInstance(0).impl = nullptr;
        device::FrontPanelIndicator::getInstance(0).impl = nullptr;

    }
};

TEST_F(FrontPanelTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setBrightness")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getBrightness")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("powerLedOn")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("powerLedOff")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setClockBrightness")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getClockBrightness")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getFrontPanelLights")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPreferences")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setPreferences")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setLED")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setBlink")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("set24HourClock")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("is24HourClock")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setClockTestPattern")));
}

TEST_F(FrontPanelDsTest, setBrightnessWIndex)
{
    ON_CALL(frontPanelTextDisplayImplStringMock, setBrightness(::testing::_, ::testing::_))
        .WillByDefault(::testing::Return());
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setBrightness"), _T("{\"brightness\": 1,\"index\": \"power_led\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}
TEST_F(FrontPanelDsTest, setBrightnessClock)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setBrightness"), _T("{\"brightness\": 1,\"index\": \"clock_led\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(FrontPanelDsTest, setBrightness)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setBrightness"), _T("{\"brightness\": 1}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}
TEST_F(FrontPanelDsTest, setBrightnessFalse)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setBrightness"), _T("{}"), response));
    EXPECT_EQ(response, string(""));
}
TEST_F(FrontPanelDsTest, setBrightnessNeg1)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setBrightness"), _T("{\"brightness\": -1}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(FrontPanelDsTest, getBrightnessWIndex)
{
    ON_CALL(frontPanelIndicatorImplStringMock, getBrightness())
        .WillByDefault(::testing::Return(50));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getBrightness"), _T("{\"index\": \"power_led\"}"), response));
    EXPECT_EQ(response, string("{\"brightness\":50,\"success\":true}"));
}

TEST_F(FrontPanelDsTest, getBrightnessOtherName)
{
    ON_CALL(frontPanelIndicatorImplStringMock, getBrightness())
        .WillByDefault(::testing::Return(50));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getBrightness"), _T("{\"index\": \"other\"}"), response));
    EXPECT_EQ(response, string("{\"brightness\":50,\"success\":true}"));
}

TEST_F(FrontPanelDsTest, getBrightnessWIndexClock)
{
    ON_CALL(frontPanelTextDisplayImplStringMock, getTextBrightness())
        .WillByDefault(::testing::Return(50));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getBrightness"), _T("{\"index\": \"clock_led\"}"), response));
    EXPECT_EQ(response, string("{\"brightness\":50,\"success\":true}"));
}


TEST_F(FrontPanelDsTest, getBrightness)
{
    ON_CALL(frontPanelIndicatorImplStringMock, getBrightness())
        .WillByDefault(::testing::Return(50));
    ON_CALL(frontPanelTextDisplayImplStringMock, getTextBrightness())
        .WillByDefault(::testing::Return(50));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getBrightness"), _T(""), response));
    EXPECT_EQ(response, string("{\"brightness\":50,\"success\":true}"));
}
TEST_F(FrontPanelDsTest, getClockBrightness)
{
    ON_CALL(frontPanelTextDisplayImplStringMock, getTextBrightness())
        .WillByDefault(::testing::Return(50));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getClockBrightness"), _T(""), response));
    EXPECT_EQ(response, string("{\"brightness\":50,\"success\":true}"));
}

TEST_F(FrontPanelDsTest, setClockBrightness)
{
    ON_CALL(frontPanelTextDisplayImplStringMock, setTextBrightness(::testing::_))
        .WillByDefault(::testing::Return());

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setClockBrightness"), _T("{\"brightness\": 1}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}
TEST_F(FrontPanelDsTest, setClockBrightnessInvalid)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setClockBrightness"), _T("{\"brightness\": -1}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(FrontPanelDsTest, getFrontPanelLights)
{
    FrontPanelIndicatorImplMock frontPanelIndicatorMock;

    device::FrontPanelIndicator indicatorList;
    indicatorList.impl = &frontPanelIndicatorMock;
    ON_CALL(frontPanelConfigImplMock, getIndicators())
        .WillByDefault(::testing::Return(device::List<device::FrontPanelIndicator>({ indicatorList })));

    ON_CALL(frontPanelIndicatorMock, getBrightnessLevels(::testing::_,::testing::_,::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](int &levels,int &min,int &max) {
                levels=1;
		min=0;
		max=2;
            }));

    FrontPanelTextDisplayImplMock frontPanelTextDisplayMock;
    
    device::FrontPanelTextDisplay displayList;
    displayList.impl = &frontPanelTextDisplayMock;
    displayList.FrontPanelIndicator::impl = &frontPanelIndicatorMock;

    ON_CALL(frontPanelConfigImplMock, getTextDisplays())
        .WillByDefault(::testing::Return(device::List<device::FrontPanelTextDisplay>({ displayList })));
    



    ON_CALL(frontPanelTextDisplayMock, getName())
        .WillByDefault(::testing::Return("Text"));

   
	int test = 0;

    ON_CALL(frontPanelConfigImplMock, getIndicators())
        .WillByDefault(::testing::Return(device::List<device::FrontPanelIndicator>({ indicatorList })));

    ON_CALL(frontPanelIndicatorMock, getBrightnessLevels(::testing::_,::testing::_,::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](int &levels,int &min,int &max) {
                levels=1;
                min=0;
                max=2;
            }));

    device::FrontPanelTextDisplay displayList2;
    displayList2.impl = &frontPanelTextDisplayMock;
    displayList2.FrontPanelIndicator::impl = &frontPanelIndicatorMock;

    ON_CALL(frontPanelConfigImplMock, getTextDisplay(test))
        .WillByDefault(::testing::ReturnRef(displayList2));

    ON_CALL(frontPanelIndicatorMock, getBrightnessLevels(::testing::_,::testing::_,::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](int &levels,int &min,int &max) {
                levels=1;
                min=0;
                max=2;
            }));


    ON_CALL(frontPanelIndicatorMock, getName())
        .WillByDefault(::testing::Return("Power"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFrontPanelLights"), _T(""), response));
    EXPECT_EQ(response, string("{\"supportedLights\":[\"power_led\",\"clock_led\"],\"supportedLightsInfo\":{\"power_led\":{\"range\":\"boolean\",\"min\":0,\"max\":2,\"colors\":[\"white\"],\"colorMode\":0},\"clock_led\":{\"range\":\"boolean\",\"min\":0,\"max\":2,\"colors\":[\"white\"],\"colorMode\":0}},\"success\":true}"));
    indicatorList.impl = nullptr;
    displayList.impl = nullptr;
    displayList2.FrontPanelIndicator::impl = nullptr;
}

TEST_F(FrontPanelDsTest, getPreferences)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPreferences"), _T(""), response));
    EXPECT_EQ(response, string("{\"preferences\":{},\"success\":true}"));
}

TEST_F(FrontPanelDsTest, is24HourClock)
{   
    device::FrontPanelTextDisplay displayList;
    displayList.impl = &frontPanelTextDisplayImplStringMock;
    displayList.FrontPanelIndicator::impl = &frontPanelIndicatorImplMock;

    std::string test = "Text";
    
    ON_CALL(frontPanelConfigImplMock, getTextDisplay(test))
        .WillByDefault(::testing::ReturnRef(displayList));
    ON_CALL(frontPanelTextDisplayImplStringMock, getCurrentTimeFormat())
        .WillByDefault(::testing::Return(dsFPD_TIME_12_HOUR));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("is24HourClock"), _T(""), response));
    EXPECT_EQ(response, string("{\"is24Hour\":false,\"success\":true}"));

    ON_CALL(frontPanelTextDisplayImplStringMock, getCurrentTimeFormat())
        .WillByDefault(::testing::Return(dsFPD_TIME_24_HOUR));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("is24HourClock"), _T(""), response));
    EXPECT_EQ(response, string("{\"is24Hour\":true,\"success\":true}"));

}

TEST_F(FrontPanelDsTest, powerLedOffPower)
{
    ON_CALL(frontPanelTextDisplayImplStringMock, setTextBrightness(::testing::_))
        .WillByDefault(::testing::Return());

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("powerLedOff"), _T("{\"index\": \"power_led\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}
TEST_F(FrontPanelDsTest, powerLedOffData)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("powerLedOff"), _T("{\"index\": \"data_led\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}
TEST_F(FrontPanelDsTest, powerLedOffRecord)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("powerLedOff"), _T("{\"index\": \"record_led\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(FrontPanelInitializedEventDsTest, powerLedOnPower)
{
FrontPanelIndicatorImplMock frontPanelIndicatorMock;

    device::FrontPanelIndicator indicatorList;
    indicatorList.impl = &frontPanelIndicatorMock;
    ON_CALL(frontPanelConfigImplMock, getIndicators())
        .WillByDefault(::testing::Return(device::List<device::FrontPanelIndicator>({indicatorList})));
    ON_CALL(frontPanelIndicatorMock, getName())
        .WillByDefault(::testing::Return("red"));

    ASSERT_TRUE(dsFrontPanelModeChange != nullptr);


    IARM_Bus_PWRMgr_EventData_t eventData;
    eventData.data.state.newState =IARM_BUS_PWRMGR_POWERSTATE_ON;
    eventData.data.state.curState =IARM_BUS_PWRMGR_POWERSTATE_STANDBY;

    handler.Subscribe(0, _T("powerModeChange"), _T("client.events.powerModeChange"), message);

    dsFrontPanelModeChange(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, &eventData , 0);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("powerLedOn"), _T("{\"index\": \"power_led\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("powerLedOn"), _T("{\"index\": \"record_led\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("powerLedOn"), _T("{\"index\": \"data_led\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));


    eventData.data.state.newState =IARM_BUS_PWRMGR_POWERSTATE_OFF;
    eventData.data.state.curState =IARM_BUS_PWRMGR_POWERSTATE_STANDBY;

    handler.Subscribe(0, _T("powerModeChange"), _T("client.events.powerModeChange"), message);

    dsFrontPanelModeChange(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, &eventData , 0);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("powerLedOn"), _T("{\"index\": \"power_led\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("powerLedOn"), _T("{\"index\": \"record_led\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("powerLedOn"), _T("{\"index\": \"data_led\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    indicatorList.impl = nullptr;

}

TEST_F(FrontPanelDsTest, set24HourClock)
{
    device::FrontPanelTextDisplay displayList;
    displayList.impl = &frontPanelTextDisplayImplStringMock;
    displayList.FrontPanelIndicator::impl = &frontPanelIndicatorImplMock;

    std::string test = "Text";

    ON_CALL(frontPanelConfigImplMock, getTextDisplay(test))
        .WillByDefault(::testing::ReturnRef(displayList));
    ON_CALL(frontPanelTextDisplayImplStringMock, getCurrentTimeFormat())
        .WillByDefault(::testing::Return(dsFPD_TIME_24_HOUR));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("set24HourClock"), _T("{\"is24Hour\": true}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    displayList.impl = nullptr;
    displayList.FrontPanelIndicator::impl = nullptr;

}

TEST_F(FrontPanelDsTest, setBlink)
{

    ON_CALL(frontPanelIndicatorImplStringMock, getBrightness())
        .WillByDefault(::testing::Return(50));
    ON_CALL(frontPanelTextDisplayImplStringMock, getTextBrightness())
        .WillByDefault(::testing::Return(50));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setBlink"), _T("{\"blinkInfo\": {\"ledIndicator\": \"power_led\", \"iterations\": 10, \"pattern\": [{\"brightness\": 50, \"duration\": 100, \"color\": \"red\", \"red\": 0, \"green\":0, \"blue\":0}]}}"), response));
   
       	EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(FrontPanelDsTest, setClockTestPattern)
{
    device::FrontPanelTextDisplay displayList;
    displayList.impl = &frontPanelTextDisplayImplStringMock;
    displayList.FrontPanelIndicator::impl = &frontPanelIndicatorImplMock;

    std::string test = "Text";

    ON_CALL(frontPanelConfigImplMock, getTextDisplay(test))
        .WillByDefault(::testing::ReturnRef(displayList));
    ON_CALL(frontPanelTextDisplayImplStringMock, getTextBrightness())
        .WillByDefault(::testing::Return(100));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setClockTestPattern"), _T("{\"show\": true, \"timeInterval\": 4}"), response));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setClockTestPattern"), _T("{\"show\": true, \"timeInterval\": 64}"), response));

    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setClockTestPattern"), _T("{\"show\": false, \"timeInterval\": 4}"), response));

    EXPECT_EQ(response, string("{\"success\":true}"));
    displayList.impl = nullptr;
    displayList.FrontPanelIndicator::impl = nullptr;

}

TEST_F(FrontPanelDsTest, setLED)
{
    ON_CALL(frontPanelTextDisplayImplStringMock, setColor())
        .WillByDefault(::testing::Return());
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setLED"), _T("{\"ledIndicator\": \"power_led\", \"brightness\": 50, \"color\": \"red\", \"red\": 0, \"green\": 0, \"blue\":0}"), response));

        EXPECT_EQ(response, string("{\"success\":true}"));
}
TEST_F(FrontPanelDsTest, setPreferences)
{

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setPreferences"), _T("{\"preferences\":{}}"), response));

        EXPECT_EQ(response, string("{\"success\":true}"));
}

