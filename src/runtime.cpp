#include "runtime.h"

// Extension-provided methods

extern "C" {
LOADER_EXPORT XRAPI_ATTR XrResult xrGetOpenGLESGraphicsRequirementsKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrGraphicsRequirementsOpenGLESKHR*          graphicsRequirements);
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateApiLayerProperties(
    uint32_t                                    propertyCapacityInput,
    uint32_t*                                   propertyCountOutput,
    XrApiLayerProperties*                       properties)
{
    if (propertyCountOutput == nullptr) {
        return XR_ERROR_VALIDATION_FAILURE;
    }

    if (propertyCapacityInput != 0 && properties == nullptr) {
        return XR_ERROR_VALIDATION_FAILURE;
    }

    // API layers not supported
    *propertyCountOutput = 0;
    return XR_SUCCESS;
}


static const XrExtensionProperties supportedExtensions[] = {
    { XR_TYPE_EXTENSION_PROPERTIES, nullptr, XR_KHR_OPENGL_ES_ENABLE_EXTENSION_NAME, 8 },
};

static constexpr uint32_t numSupportedExtensions = sizeof(supportedExtensions)/sizeof(supportedExtensions[0]);

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateInstanceExtensionProperties(
    const char*                                 layerName,
    uint32_t                                    propertyCapacityInput,
    uint32_t*                                   propertyCountOutput,
    XrExtensionProperties*                      properties)
{
    if (layerName != nullptr) {
        return XR_ERROR_API_LAYER_NOT_PRESENT;
    }
    if (propertyCountOutput == nullptr) {
        return XR_ERROR_VALIDATION_FAILURE;
    }
    if (propertyCapacityInput == 0) {
        *propertyCountOutput = numSupportedExtensions;
        return XR_SUCCESS;
    }
    if (properties == nullptr) {
        return XR_ERROR_VALIDATION_FAILURE;
    }
    if (propertyCapacityInput < numSupportedExtensions) {
        return XR_ERROR_SIZE_INSUFFICIENT;
    }
    *propertyCountOutput = numSupportedExtensions;
    for (uint32_t i = 0; i < numSupportedExtensions; ++i) {
        if (properties[i].type != XR_TYPE_EXTENSION_PROPERTIES) {
            return XR_ERROR_VALIDATION_FAILURE;
        }
        strncpy(properties[i].extensionName, supportedExtensions[i].extensionName, XR_MAX_EXTENSION_NAME_SIZE);
        properties[i].extensionVersion = supportedExtensions[i].extensionVersion;
    }
    return XR_SUCCESS;
}


LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrCreateInstance(
    const XrInstanceCreateInfo*                 info,
    XrInstance*                                 instance)
{
    if (info == nullptr || info->type != XR_TYPE_INSTANCE_CREATE_INFO)
        return XR_ERROR_VALIDATION_FAILURE;
    if (instance == nullptr)
        return XR_ERROR_VALIDATION_FAILURE;
    if (createFlags != 0)
        return XR_ERROR_VALIDATION_FAILURE;
    if (info->enabledApiLayerCount != 0)
        return XR_ERROR_API_LAYER_NOT_PRESENT;

    // The graphics extension must be enabled.
    bool eglEnabled = false;
    for (uint32_t i = 0; i < info->enabledExtensionCount; ++i) {
        if (strcmp(info->enabledExtensionNames[i], XR_KHR_OPENGL_ES_ENABLE_EXTENSION_NAME) == 0) {
            eglEnabled = true;
        } else {
            return XR_ERROR_EXTENSION_NOT_PRESENT;
        }
    }
    if (!eglEnabled) {
        return XR_ERROR_INITIALIZATION_FAILED;
    }
    if (WebInstance::get() != nullptr) {
        return XR_ERROR_LIMIT_REACHED;
    }
    WebInstance::make(info->applicationInfo);
    *instance = WebInstance::get().getHandle();
    return XR_SUCCESS;
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrDestroyInstance(
    XrInstance                                  instance)
{
    auto inst = WebInstance::getFromHandle(instance);
    if (inst == nullptr)
        return XR_ERROR_HANDLE_INVALID;
    inst->Destroy();
    return XR_SUCCESS;
}

#define FOREACH_XR_METHOD(X) \
  X(xrEnumerateApiLayerProperties) \
  X(xrEnumerateInstanceExtensionProperties) \
  X(xrCreateInstance) \
  X(xrDestroyInstance) \
  X(xrGetInstanceProcAddr) \
  X(xrGetInstanceProperties) \
  X(xrPollEvent) \
  X(xrResultToString) \
  X(xrStructureTypeToString) \
  X(xrGetSystem) \
  X(xrGetSystemProperties) \
  X(xrEnumerateEnvironmentBlendModes) \
  X(xrCreateSession) \
  X(xrDestroySession) \
  X(xrEnumerateReferenceSpaces) \
  X(xrCreateReferenceSpace) \
  X(xrGetReferenceSpaceBoundsRect) \
  X(xrCreateActionSpace) \
  X(xrLocateSpace) \
  X(xrDestroySpace) \
  X(xrEnumerateViewConfigurations) \
  X(xrGetViewConfigurationProperties) \
  X(xrEnumerateViewConfigurationViews) \
  X(xrEnumerateSwapchainFormats) \
  X(xrCreateSwapchain) \
  X(xrDestroySwapchain) \
  X(xrEnumerateSwapchainImages) \
  X(xrAcquireSwapchainImage) \
  X(xrWaitSwapchainImage) \
  X(xrReleaseSwapchainImage) \
  X(xrBeginSession) \
  X(xrEndSession) \
  X(xrRequestExitSession) \
  X(xrWaitFrame) \
  X(xrBeginFrame) \
  X(xrEndFrame) \
  X(xrLocateViews) \
  X(xrStringToPath) \
  X(xrPathToString) \
  X(xrCreateActionSet) \
  X(xrDestroyActionSet) \
  X(xrCreateAction) \
  X(xrDestroyAction) \
  X(xrSuggestInteractionProfileBindings) \
  X(xrAttachSessionActionSets) \
  X(xrGetCurrentInteractionProfile) \
  X(xrGetActionStateBoolean) \
  X(xrGetActionStateFloat) \
  X(xrGetActionStateVector2f) \
  X(xrGetActionStatePose) \
  X(xrSyncActions) \
  X(xrEnumerateBoundSourcesForAction) \
  X(xrGetInputSourceLocalizedName) \
  X(xrApplyHapticFeedback) \
  X(xrStopHapticFeedback)

#define HANDLE(_fname) \
    if (strcmp(name, #_fname) == 0) { \
        *function = _fname; \
        return XR_SUCCESS; \
    }

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetInstanceProcAddr(
    XrInstance                                  instance,
    const char*                                 name,
    PFN_xrVoidFunction*                         function)
{
    if (name == nullptr || function == nullptr)
        return XR_ERROR_VALIDATION_FAILURE;

    if (instance == XR_NULL_HANDLE) {
        HANDLE(xrEnumerateInstanceExtensionProperties);
        HANDLE(xrEnumerateApiLayerProperties);
        HANDLE(xrCreateInstance);
        *function = nullptr;
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    }
    auto inst = WebInstance::getFromHandle(instance);
    if (inst == nullptr) {
        return XR_ERROR_HANDLE_INVALID;
    }

    HANDLE(xrGetOpenGLESGraphicsRequirementsKHR);
    FOREACH_XR_METHOD(HANDLE)
    *function = nullptr;
    return XR_ERROR_FUNCTION_UNSUPPORTED;
}

////////////////////////////////////////////////////////////////////////////////////
// Runtime methods

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetInstanceProperties(
    XrInstance                                  instance,
    XrInstanceProperties*                       instanceProperties)
{
    
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrPollEvent(
    XrInstance                                  instance,
    XrEventDataBuffer*                          eventData)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrResultToString(
    XrInstance                                  instance,
    XrResult                                    value,
    char                                        buffer[XR_MAX_RESULT_STRING_SIZE])
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrStructureTypeToString(
    XrInstance                                  instance,
    XrStructureType                             value,
    char                                        buffer[XR_MAX_STRUCTURE_NAME_SIZE])
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetSystem(
    XrInstance                                  instance,
    const XrSystemGetInfo*                      getInfo,
    XrSystemId*                                 systemId)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetSystemProperties(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrSystemProperties*                         properties)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateEnvironmentBlendModes(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrViewConfigurationType                     viewConfigurationType,
    uint32_t                                    environmentBlendModeCapacityInput,
    uint32_t*                                   environmentBlendModeCountOutput,
    XrEnvironmentBlendMode*                     environmentBlendModes)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrCreateSession(
    XrInstance                                  instance,
    const XrSessionCreateInfo*                  createInfo,
    XrSession*                                  session)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrDestroySession(
    XrSession                                   session)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateReferenceSpaces(
    XrSession                                   session,
    uint32_t                                    spaceCapacityInput,
    uint32_t*                                   spaceCountOutput,
    XrReferenceSpaceType*                       spaces)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrCreateReferenceSpace(
    XrSession                                   session,
    const XrReferenceSpaceCreateInfo*           createInfo,
    XrSpace*                                    space)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetReferenceSpaceBoundsRect(
    XrSession                                   session,
    XrReferenceSpaceType                        referenceSpaceType,
    XrExtent2Df*                                bounds)
{
    
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrCreateActionSpace(
    XrSession                                   session,
    const XrActionSpaceCreateInfo*              createInfo,
    XrSpace*                                    space)
{
    return XR_ERROR_RUNTIME_FAILURE;
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrLocateSpace(
    XrSpace                                     space,
    XrSpace                                     baseSpace,
    XrTime                                      time,
    XrSpaceLocation*                            location)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrDestroySpace(
    XrSpace                                     space)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateViewConfigurations(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    uint32_t                                    viewConfigurationTypeCapacityInput,
    uint32_t*                                   viewConfigurationTypeCountOutput,
    XrViewConfigurationType*                    viewConfigurationTypes)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetViewConfigurationProperties(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrViewConfigurationType                     viewConfigurationType,
    XrViewConfigurationProperties*              configurationProperties)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateViewConfigurationViews(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrViewConfigurationType                     viewConfigurationType,
    uint32_t                                    viewCapacityInput,
    uint32_t*                                   viewCountOutput,
    XrViewConfigurationView*                    views)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateSwapchainFormats(
    XrSession                                   session,
    uint32_t                                    formatCapacityInput,
    uint32_t*                                   formatCountOutput,
    int64_t*                                    formats)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrCreateSwapchain(
    XrSession                                   session,
    const XrSwapchainCreateInfo*                createInfo,
    XrSwapchain*                                swapchain)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrDestroySwapchain(
    XrSwapchain                                 swapchain)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateSwapchainImages(
    XrSwapchain                                 swapchain,
    uint32_t                                    imageCapacityInput,
    uint32_t*                                   imageCountOutput,
    XrSwapchainImageBaseHeader*                 images)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrAcquireSwapchainImage(
    XrSwapchain                                 swapchain,
    const XrSwapchainImageAcquireInfo*          acquireInfo,
    uint32_t*                                   index)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrWaitSwapchainImage(
    XrSwapchain                                 swapchain,
    const XrSwapchainImageWaitInfo*             waitInfo)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrReleaseSwapchainImage(
    XrSwapchain                                 swapchain,
    const XrSwapchainImageReleaseInfo*          releaseInfo)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrBeginSession(
    XrSession                                   session,
    const XrSessionBeginInfo*                   beginInfo)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEndSession(
    XrSession                                   session)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrRequestExitSession(
    XrSession                                   session)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrWaitFrame(
    XrSession                                   session,
    const XrFrameWaitInfo*                      frameWaitInfo,
    XrFrameState*                               frameState)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrBeginFrame(
    XrSession                                   session,
    const XrFrameBeginInfo*                     frameBeginInfo)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEndFrame(
    XrSession                                   session,
    const XrFrameEndInfo*                       frameEndInfo)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrLocateViews(
    XrSession                                   session,
    const XrViewLocateInfo*                     viewLocateInfo,
    XrViewState*                                viewState,
    uint32_t                                    viewCapacityInput,
    uint32_t*                                   viewCountOutput,
    XrView*                                     views)
{
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrStringToPath(
    XrInstance                                  instance,
    const char*                                 pathString,
    XrPath*                                     path)
{
    return XR_ERROR_RUNTIME_FAILURE;
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrPathToString(
    XrInstance                                  instance,
    XrPath                                      path,
    uint32_t                                    bufferCapacityInput,
    uint32_t*                                   bufferCountOutput,
    char*                                       buffer)
{
    return XR_ERROR_RUNTIME_FAILURE;
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrCreateActionSet(
    XrInstance                                  instance,
    const XrActionSetCreateInfo*                createInfo,
    XrActionSet*                                actionSet)
{
    return XR_ERROR_RUNTIME_FAILURE;
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrDestroyActionSet(
    XrActionSet                                 actionSet)
{
    return XR_ERROR_RUNTIME_FAILURE;
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrCreateAction(
    XrActionSet                                 actionSet,
    const XrActionCreateInfo*                   createInfo,
    XrAction*                                   action)
{
    return XR_ERROR_RUNTIME_FAILURE;
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrDestroyAction(
    XrAction                                    action)
{
    return XR_ERROR_RUNTIME_FAILURE;
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrSuggestInteractionProfileBindings(
    XrInstance                                  instance,
    const XrInteractionProfileSuggestedBinding* suggestedBindings)
{
    return XR_ERROR_RUNTIME_FAILURE;
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrAttachSessionActionSets(
    XrSession                                   session,
    const XrSessionActionSetsAttachInfo*        attachInfo)
{
    return XR_ERROR_RUNTIME_FAILURE;
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetCurrentInteractionProfile(
    XrSession                                   session,
    XrPath                                      topLevelUserPath,
    XrInteractionProfileState*                  interactionProfile)
{
    return XR_ERROR_RUNTIME_FAILURE;
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetActionStateBoolean(
    XrSession                                   session,
    const XrActionStateGetInfo*                 getInfo,
    XrActionStateBoolean*                       state)
{
    return XR_ERROR_RUNTIME_FAILURE;
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetActionStateFloat(
    XrSession                                   session,
    const XrActionStateGetInfo*                 getInfo,
    XrActionStateFloat*                         state)
{
    return XR_ERROR_RUNTIME_FAILURE;
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetActionStateVector2f(
    XrSession                                   session,
    const XrActionStateGetInfo*                 getInfo,
    XrActionStateVector2f*                      state)
{
    return XR_ERROR_RUNTIME_FAILURE;
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetActionStatePose(
    XrSession                                   session,
    const XrActionStateGetInfo*                 getInfo,
    XrActionStatePose*                          state)
{
    return XR_ERROR_RUNTIME_FAILURE;
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrSyncActions(
    XrSession                                   session,
    const XrActionsSyncInfo*                    syncInfo)
{
    return XR_ERROR_RUNTIME_FAILURE;
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateBoundSourcesForAction(
    XrSession                                   session,
    const XrBoundSourcesForActionEnumerateInfo* enumerateInfo,
    uint32_t                                    sourceCapacityInput,
    uint32_t*                                   sourceCountOutput,
    XrPath*                                     sources)
{
    return XR_ERROR_RUNTIME_FAILURE;
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetInputSourceLocalizedName(
    XrSession                                   session,
    const XrInputSourceLocalizedNameGetInfo*    getInfo,
    uint32_t                                    bufferCapacityInput,
    uint32_t*                                   bufferCountOutput,
    char*                                       buffer)
{
    return XR_ERROR_RUNTIME_FAILURE;
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrApplyHapticFeedback(
    XrSession                                   session,
    const XrHapticActionInfo*                   hapticActionInfo,
    const XrHapticBaseHeader*                   hapticFeedback)
{
    return XR_ERROR_RUNTIME_FAILURE;
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrStopHapticFeedback(
    XrSession                                   session,
    const XrHapticActionInfo*                   hapticActionInfo)
{
    return XR_ERROR_RUNTIME_FAILURE;
}
