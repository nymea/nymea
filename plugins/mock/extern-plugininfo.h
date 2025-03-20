/* This file is generated by the nymea build system. Any changes to this file will *
 * be lost. If you want to change this file, edit the plugin's json file.          *
 *                                                                                 *
 * NOTE: This file can be included only once per plugin. If you need to access     *
 * definitions from this file in multiple source files, use                        *
 * #include extern-plugininfo.h                                                    *
 * instead and re-run qmake.                                                       */

#ifndef EXTERNPLUGININFO_H
#define EXTERNPLUGININFO_H

#include "typeutils.h"

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(dcMock)

extern PluginId pluginId;
extern ParamTypeId mockPluginConfigParamIntParamTypeId;
extern ParamTypeId mockPluginConfigParamBoolParamTypeId;
extern VendorId nymeaVendorId;
extern ThingClassId mockThingClassId;
extern ParamTypeId mockThingHttpportParamTypeId;
extern ParamTypeId mockThingAsyncParamTypeId;
extern ParamTypeId mockThingBrokenParamTypeId;
extern ParamTypeId mockSettingsSetting1ParamTypeId;
extern ParamTypeId mockSettingsIntStateWithLimitsMinValueParamTypeId;
extern ParamTypeId mockSettingsIntStateWithLimitsMaxValueParamTypeId;
extern ParamTypeId mockDiscoveryResultCountParamTypeId;
extern StateTypeId mockIntStateTypeId;
extern StateTypeId mockIntWithLimitsStateTypeId;
extern StateTypeId mockBoolStateTypeId;
extern StateTypeId mockDoubleStateTypeId;
extern StateTypeId mockBatteryLevelStateTypeId;
extern StateTypeId mockBatteryCriticalStateTypeId;
extern StateTypeId mockPowerStateTypeId;
extern StateTypeId mockConnectedStateTypeId;
extern StateTypeId mockSignalStrengthStateTypeId;
extern StateTypeId mockUpdateStatusStateTypeId;
extern StateTypeId mockCurrentVersionStateTypeId;
extern StateTypeId mockAvailableVersionStateTypeId;
extern EventTypeId mockEvent1EventTypeId;
extern EventTypeId mockEvent2EventTypeId;
extern ParamTypeId mockEvent2EventIntParamParamTypeId;
extern EventTypeId mockPressedEventTypeId;
extern ParamTypeId mockPressedEventButtonNameParamTypeId;
extern ActionTypeId mockIntWithLimitsActionTypeId;
extern ParamTypeId mockIntWithLimitsActionIntWithLimitsParamTypeId;
extern ActionTypeId mockBatteryLevelActionTypeId;
extern ParamTypeId mockBatteryLevelActionBatteryLevelParamTypeId;
extern ActionTypeId mockPowerActionTypeId;
extern ParamTypeId mockPowerActionPowerParamTypeId;
extern ActionTypeId mockSignalStrengthActionTypeId;
extern ParamTypeId mockSignalStrengthActionSignalStrengthParamTypeId;
extern ActionTypeId mockUpdateStatusActionTypeId;
extern ParamTypeId mockUpdateStatusActionUpdateStatusParamTypeId;
extern ActionTypeId mockWithParamsActionTypeId;
extern ParamTypeId mockWithParamsActionParam1ParamTypeId;
extern ParamTypeId mockWithParamsActionParam2ParamTypeId;
extern ParamTypeId mockWithParamsActionParam3ParamTypeId;
extern ActionTypeId mockWithoutParamsActionTypeId;
extern ActionTypeId mockAsyncActionTypeId;
extern ActionTypeId mockFailingActionTypeId;
extern ActionTypeId mockAsyncFailingActionTypeId;
extern ActionTypeId mockPerformUpdateActionTypeId;
extern ActionTypeId mockPressButtonActionTypeId;
extern ParamTypeId mockPressButtonActionButtonNameParamTypeId;
extern ActionTypeId mockAddToFavoritesBrowserItemActionTypeId;
extern ActionTypeId mockRemoveFromFavoritesBrowserItemActionTypeId;
extern ThingClassId autoMockThingClassId;
extern ParamTypeId autoMockThingHttpportParamTypeId;
extern ParamTypeId autoMockThingAsyncParamTypeId;
extern ParamTypeId autoMockThingBrokenParamTypeId;
extern ParamTypeId autoMockSettingsMockSettingParamTypeId;
extern StateTypeId autoMockIntStateTypeId;
extern StateTypeId autoMockBoolValueStateTypeId;
extern EventTypeId autoMockEvent1EventTypeId;
extern EventTypeId autoMockEvent2EventTypeId;
extern ParamTypeId autoMockEvent2EventIntParamParamTypeId;
extern ActionTypeId autoMockWithParamsActionTypeId;
extern ParamTypeId autoMockWithParamsActionMockActionParam1ParamTypeId;
extern ParamTypeId autoMockWithParamsActionMockActionParam2ParamTypeId;
extern ActionTypeId autoMockMockActionNoParmsActionTypeId;
extern ActionTypeId autoMockMockActionAsyncActionTypeId;
extern ActionTypeId autoMockMockActionBrokenActionTypeId;
extern ActionTypeId autoMockMockActionAsyncBrokenActionTypeId;
extern ThingClassId pushButtonMockThingClassId;
extern ParamTypeId pushButtonMockDiscoveryResultCountParamTypeId;
extern StateTypeId pushButtonMockColorStateTypeId;
extern StateTypeId pushButtonMockPercentageStateTypeId;
extern StateTypeId pushButtonMockAllowedValuesStateTypeId;
extern StateTypeId pushButtonMockDoubleStateTypeId;
extern StateTypeId pushButtonMockBoolStateTypeId;
extern ActionTypeId pushButtonMockColorActionTypeId;
extern ParamTypeId pushButtonMockColorActionColorParamTypeId;
extern ActionTypeId pushButtonMockPercentageActionTypeId;
extern ParamTypeId pushButtonMockPercentageActionPercentageParamTypeId;
extern ActionTypeId pushButtonMockAllowedValuesActionTypeId;
extern ParamTypeId pushButtonMockAllowedValuesActionAllowedValuesParamTypeId;
extern ActionTypeId pushButtonMockDoubleActionTypeId;
extern ParamTypeId pushButtonMockDoubleActionDoubleParamTypeId;
extern ActionTypeId pushButtonMockBoolActionTypeId;
extern ParamTypeId pushButtonMockBoolActionBoolParamTypeId;
extern ActionTypeId pushButtonMockTimeoutActionTypeId;
extern ThingClassId displayPinMockThingClassId;
extern ParamTypeId displayPinMockThingPinParamTypeId;
extern ParamTypeId displayPinMockDiscoveryResultCountParamTypeId;
extern StateTypeId displayPinMockColorStateTypeId;
extern StateTypeId displayPinMockPercentageStateTypeId;
extern StateTypeId displayPinMockAllowedValuesStateTypeId;
extern StateTypeId displayPinMockDoubleStateTypeId;
extern StateTypeId displayPinMockBoolStateTypeId;
extern ActionTypeId displayPinMockColorActionTypeId;
extern ParamTypeId displayPinMockColorActionColorParamTypeId;
extern ActionTypeId displayPinMockPercentageActionTypeId;
extern ParamTypeId displayPinMockPercentageActionPercentageParamTypeId;
extern ActionTypeId displayPinMockAllowedValuesActionTypeId;
extern ParamTypeId displayPinMockAllowedValuesActionAllowedValuesParamTypeId;
extern ActionTypeId displayPinMockDoubleActionTypeId;
extern ParamTypeId displayPinMockDoubleActionDoubleParamTypeId;
extern ActionTypeId displayPinMockBoolActionTypeId;
extern ParamTypeId displayPinMockBoolActionBoolParamTypeId;
extern ActionTypeId displayPinMockTimeoutActionTypeId;
extern ThingClassId parentMockThingClassId;
extern StateTypeId parentMockBoolValueStateTypeId;
extern EventTypeId parentMockEvent1EventTypeId;
extern ActionTypeId parentMockBoolValueActionTypeId;
extern ParamTypeId parentMockBoolValueActionBoolValueParamTypeId;
extern ThingClassId childMockThingClassId;
extern StateTypeId childMockBoolValueStateTypeId;
extern EventTypeId childMockEvent1EventTypeId;
extern ActionTypeId childMockBoolValueActionTypeId;
extern ParamTypeId childMockBoolValueActionBoolValueParamTypeId;
extern ThingClassId inputTypeMockThingClassId;
extern ParamTypeId inputTypeMockThingTextLineParamTypeId;
extern ParamTypeId inputTypeMockThingTextAreaParamTypeId;
extern ParamTypeId inputTypeMockThingPasswordParamTypeId;
extern ParamTypeId inputTypeMockThingSearchParamTypeId;
extern ParamTypeId inputTypeMockThingMailParamTypeId;
extern ParamTypeId inputTypeMockThingIp4ParamTypeId;
extern ParamTypeId inputTypeMockThingIp6ParamTypeId;
extern ParamTypeId inputTypeMockThingUrlParamTypeId;
extern ParamTypeId inputTypeMockThingMacParamTypeId;
extern ParamTypeId inputTypeMockSettingsBoolParamTypeId;
extern ParamTypeId inputTypeMockSettingsIntParamTypeId;
extern ParamTypeId inputTypeMockSettingsIntWithLimitsParamTypeId;
extern ParamTypeId inputTypeMockSettingsDoubleParamTypeId;
extern ParamTypeId inputTypeMockSettingsDoubleWithLimitsParamTypeId;
extern ParamTypeId inputTypeMockSettingsStringParamTypeId;
extern ParamTypeId inputTypeMockSettingsColorParamTypeId;
extern StateTypeId inputTypeMockBoolStateTypeId;
extern StateTypeId inputTypeMockWritableBoolStateTypeId;
extern StateTypeId inputTypeMockIntStateTypeId;
extern StateTypeId inputTypeMockWritableIntStateTypeId;
extern StateTypeId inputTypeMockWritableIntMinMaxStateTypeId;
extern StateTypeId inputTypeMockUintStateTypeId;
extern StateTypeId inputTypeMockWritableUIntStateTypeId;
extern StateTypeId inputTypeMockWritableUIntMinMaxStateTypeId;
extern StateTypeId inputTypeMockDoubleStateTypeId;
extern StateTypeId inputTypeMockWritableDoubleStateTypeId;
extern StateTypeId inputTypeMockWritableDoubleMinMaxStateTypeId;
extern StateTypeId inputTypeMockStringStateTypeId;
extern StateTypeId inputTypeMockWritableStringStateTypeId;
extern StateTypeId inputTypeMockWritableStringSelectionStateTypeId;
extern StateTypeId inputTypeMockColorStateTypeId;
extern StateTypeId inputTypeMockWritableColorStateTypeId;
extern StateTypeId inputTypeMockTimeStateTypeId;
extern StateTypeId inputTypeMockWritableTimeStateTypeId;
extern StateTypeId inputTypeMockTimestampIntStateTypeId;
extern StateTypeId inputTypeMockWritableTimestampIntStateTypeId;
extern StateTypeId inputTypeMockTimestampUIntStateTypeId;
extern StateTypeId inputTypeMockWritableTimestampUIntStateTypeId;
extern StateTypeId inputTypeMockLocalizedListStateTypeId;
extern ActionTypeId inputTypeMockWritableBoolActionTypeId;
extern ParamTypeId inputTypeMockWritableBoolActionWritableBoolParamTypeId;
extern ActionTypeId inputTypeMockWritableIntActionTypeId;
extern ParamTypeId inputTypeMockWritableIntActionWritableIntParamTypeId;
extern ActionTypeId inputTypeMockWritableIntMinMaxActionTypeId;
extern ParamTypeId inputTypeMockWritableIntMinMaxActionWritableIntMinMaxParamTypeId;
extern ActionTypeId inputTypeMockWritableUIntActionTypeId;
extern ParamTypeId inputTypeMockWritableUIntActionWritableUIntParamTypeId;
extern ActionTypeId inputTypeMockWritableUIntMinMaxActionTypeId;
extern ParamTypeId inputTypeMockWritableUIntMinMaxActionWritableUIntMinMaxParamTypeId;
extern ActionTypeId inputTypeMockWritableDoubleActionTypeId;
extern ParamTypeId inputTypeMockWritableDoubleActionWritableDoubleParamTypeId;
extern ActionTypeId inputTypeMockWritableDoubleMinMaxActionTypeId;
extern ParamTypeId inputTypeMockWritableDoubleMinMaxActionWritableDoubleMinMaxParamTypeId;
extern ActionTypeId inputTypeMockWritableStringActionTypeId;
extern ParamTypeId inputTypeMockWritableStringActionWritableStringParamTypeId;
extern ActionTypeId inputTypeMockWritableStringSelectionActionTypeId;
extern ParamTypeId inputTypeMockWritableStringSelectionActionWritableStringSelectionParamTypeId;
extern ActionTypeId inputTypeMockWritableColorActionTypeId;
extern ParamTypeId inputTypeMockWritableColorActionWritableColorParamTypeId;
extern ActionTypeId inputTypeMockWritableTimeActionTypeId;
extern ParamTypeId inputTypeMockWritableTimeActionWritableTimeParamTypeId;
extern ActionTypeId inputTypeMockWritableTimestampIntActionTypeId;
extern ParamTypeId inputTypeMockWritableTimestampIntActionWritableTimestampIntParamTypeId;
extern ActionTypeId inputTypeMockWritableTimestampUIntActionTypeId;
extern ParamTypeId inputTypeMockWritableTimestampUIntActionWritableTimestampUIntParamTypeId;
extern ActionTypeId inputTypeMockLocalizedListActionTypeId;
extern ParamTypeId inputTypeMockLocalizedListActionLocalizedListParamTypeId;
extern ThingClassId oAuthGoogleMockThingClassId;
extern ThingClassId oAuthSonosMockThingClassId;
extern ThingClassId userAndPassMockThingClassId;
extern ThingClassId genericIoMockThingClassId;
extern StateTypeId genericIoMockDigitalInput1StateTypeId;
extern StateTypeId genericIoMockDigitalInput2StateTypeId;
extern StateTypeId genericIoMockDigitalOutput1StateTypeId;
extern StateTypeId genericIoMockDigitalOutput2StateTypeId;
extern StateTypeId genericIoMockAnalogInput1StateTypeId;
extern StateTypeId genericIoMockAnalogInput2StateTypeId;
extern StateTypeId genericIoMockAnalogOutput1StateTypeId;
extern StateTypeId genericIoMockAnalogOutput2StateTypeId;
extern ActionTypeId genericIoMockDigitalOutput1ActionTypeId;
extern ParamTypeId genericIoMockDigitalOutput1ActionDigitalOutput1ParamTypeId;
extern ActionTypeId genericIoMockDigitalOutput2ActionTypeId;
extern ParamTypeId genericIoMockDigitalOutput2ActionDigitalOutput2ParamTypeId;
extern ActionTypeId genericIoMockAnalogInput1ActionTypeId;
extern ParamTypeId genericIoMockAnalogInput1ActionAnalogInput1ParamTypeId;
extern ActionTypeId genericIoMockAnalogOutput1ActionTypeId;
extern ParamTypeId genericIoMockAnalogOutput1ActionAnalogOutput1ParamTypeId;
extern ActionTypeId genericIoMockAnalogOutput2ActionTypeId;
extern ParamTypeId genericIoMockAnalogOutput2ActionAnalogOutput2ParamTypeId;
extern ThingClassId virtualIoLightMockThingClassId;
extern StateTypeId virtualIoLightMockPowerStateTypeId;
extern ActionTypeId virtualIoLightMockPowerActionTypeId;
extern ParamTypeId virtualIoLightMockPowerActionPowerParamTypeId;
extern ThingClassId virtualIoTemperatureSensorMockThingClassId;
extern ParamTypeId virtualIoTemperatureSensorMockSettingsMinTempParamTypeId;
extern ParamTypeId virtualIoTemperatureSensorMockSettingsMaxTempParamTypeId;
extern StateTypeId virtualIoTemperatureSensorMockInputStateTypeId;
extern StateTypeId virtualIoTemperatureSensorMockTemperatureStateTypeId;
extern ActionTypeId virtualIoTemperatureSensorMockInputActionTypeId;
extern ParamTypeId virtualIoTemperatureSensorMockInputActionInputParamTypeId;
extern ThingClassId networkDeviceMockThingClassId;
extern ParamTypeId networkDeviceMockThingMacAddressParamTypeId;
extern ParamTypeId networkDeviceMockThingHostNameParamTypeId;
extern ParamTypeId networkDeviceMockThingAddressParamTypeId;
extern ParamTypeId networkDeviceMockDiscoveryResultTypeParamTypeId;

#endif // EXTERNPLUGININFO_H
