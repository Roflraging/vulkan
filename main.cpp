#include <cstdio>
#include <cstdlib>
#include <malloc.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <Windows.h>

void* VulkanAlignedAlloc(void* userdata, size_t bytes, size_t alignment, VkSystemAllocationScope alloc_scope)
{
  return _aligned_malloc(bytes, alignment);
}

void* VulkanRealloc(void* userdata, void* ptr, size_t bytes, size_t alignment, VkSystemAllocationScope alloc_scope)
{
  return _aligned_realloc(ptr, bytes, alignment);
}

void VulkanFree(void* userdata, void* ptr)
{
  return _aligned_free(ptr);
}

void VulkanInternalAllocNotify(void* userdata, size_t bytes, VkInternalAllocationType alloc_type, VkSystemAllocationScope alloc_scope)
{
}

void VulkanInternalFreeNotify(void* userdata, size_t bytes, VkInternalAllocationType alloc_type, VkSystemAllocationScope alloc_scope)
{
}

#define VK_CHECK(vk_result)  do { VkResult result = (vk_result); if (result != VK_SUCCESS) { printf("%s:%d got %d!\n", __FILE__, __LINE__, result); getchar(); return 1; } } while(0)
#define ARRAY_COUNT(a)  (sizeof(a) / sizeof(a[0]))

const char* const g_EnabledExtensions[] =
{
  "VK_KHR_win32_surface",
};

int main()
{
  VkInstanceCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  info.enabledExtensionCount = ARRAY_COUNT(g_EnabledExtensions);
  info.ppEnabledExtensionNames = g_EnabledExtensions;

  VkAllocationCallbacks callbacks = {};
  callbacks.pfnAllocation = VulkanAlignedAlloc;
  callbacks.pfnReallocation = VulkanRealloc;
  callbacks.pfnFree = VulkanFree;
  callbacks.pfnInternalAllocation = VulkanInternalAllocNotify;
  callbacks.pfnInternalFree = VulkanInternalFreeNotify;

  VkInstance instance = {};
  VK_CHECK(vkCreateInstance(&info, &callbacks, &instance));

  uint32_t num_physical_devices = 8;
  VkPhysicalDevice physical_devices[8] = {};
  VK_CHECK(vkEnumeratePhysicalDevices(instance, &num_physical_devices, physical_devices));
  printf("%u physical device(s) found!\n", num_physical_devices);

  for (uint32_t i = 0; i < num_physical_devices; ++i)
  {
    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(physical_devices[i], &properties);
    printf("physical_devices[%u]:\n", i);
    printf("  Vulkan API %u.%u.%u\n", VK_VERSION_MAJOR(properties.apiVersion), VK_VERSION_MINOR(properties.apiVersion), VK_VERSION_PATCH(properties.apiVersion));
    printf("  Vulkan driver version: %u\n", properties.driverVersion);
    printf("  Vendor: %u\n", properties.vendorID);
    printf("  Device: %u\n", properties.deviceID);
    printf("  Device type: %u\n", properties.deviceType);
    printf("  Device name: %s\n", properties.deviceName);
    printf("\n");
  }

  uint32_t num_queue_properties = 16;
  VkQueueFamilyProperties queue_properties[16] = {};
  vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[0], &num_queue_properties, queue_properties);

  for (uint32_t i = 0; i < num_queue_properties; ++i)
  {
    printf("queue_properties[%u]:\n", i);
    printf("  Flags: %u\n", queue_properties[i].queueFlags);
    printf("  Queue count: %u\n", queue_properties[i].queueCount);
    printf("  Min image transfer granularity: (%u, %u, %u)\n", queue_properties[i].minImageTransferGranularity.width, queue_properties[i].minImageTransferGranularity.height, queue_properties[i].minImageTransferGranularity.depth);
  }

  float queue_priorities[32] = {};
  VkDeviceQueueCreateInfo queue_create_info = {};
  queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_create_info.queueFamilyIndex = 0;
  queue_create_info.queueCount = queue_properties[0].queueCount;
  queue_create_info.pQueuePriorities = queue_priorities;

  VkDeviceCreateInfo device_create_info = {};
  device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_create_info.queueCreateInfoCount = 1;
  device_create_info.pQueueCreateInfos = &queue_create_info;
  VkDevice device = {};
  VK_CHECK(vkCreateDevice(physical_devices[0], &device_create_info, &callbacks, &device));

  VkWin32SurfaceCreateInfoKHR surface_create_info = {};
  surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  //surface_create_info.hinstance = ; what to do?
  surface_create_info.hwnd = GetActiveWindow();
  VkSurfaceKHR surface = {};
  VK_CHECK(vkCreateWin32SurfaceKHR(instance, &surface_create_info, &callbacks, &surface));

  vkDestroySurfaceKHR(instance, surface, &callbacks);
  vkDestroyDevice(device, &callbacks);
  vkDestroyInstance(instance, &callbacks);
  printf("Vulkan succeeded!\n");
  getchar();

  return 0;
}