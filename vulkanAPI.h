#pragma once
#include "Singleton.h"
#include "componentManager.h"
struct QueueFamilyIndices;
struct SwapChainSupportDetails;
class Transform;
struct VertexData {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoord;

	VertexData(){}

	VertexData(const glm::vec3& p, const glm::vec3& n, const glm::vec2& t)
		: pos(p), normal(n), texCoord(t)
	{}
	static VkVertexInputBindingDescription getBindingDescription();
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescription();
	bool operator==(const VertexData& other) const {
		return pos == other.pos && normal == other.normal && texCoord == other.texCoord;
	}
};

struct MeshData
{
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
	//Material *mat;
};

struct InstanceData {
	glm::vec3 instancePos;
	static VkVertexInputBindingDescription getBindingDescription();
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescription();
};

struct UniformBufferObject {
	//alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct DynamicUBO {
	glm::mat4 *model = nullptr;
	static size_t DU_Alignment;
	std::unordered_map<glm::mat4*, Transform*> inversePtr;
	size_t lastIndex = 0;

	glm::mat4* GetAvailableModel(Transform* pTr)
	{
		glm::mat4* m = (glm::mat4*)(((uint64_t)model + (lastIndex * DU_Alignment)));
		inversePtr[m] = pTr;
		lastIndex++;
		return m;
	}

	void DeleteModel(glm::mat4* m)
	{
		glm::mat4* lastModel = (glm::mat4*)(((uint64_t)model + ((lastIndex-1) * DU_Alignment)));

		if (m != lastModel) {
			*m = *lastModel;
			inversePtr[lastModel]->transMatrix = m;
			inversePtr[m] = inversePtr[lastModel];
			inversePtr.erase(lastModel);
		}
		*lastModel = glm::mat4(1.0f);
		lastIndex--;
	}
};

class VulkanAPI:public Singleton<VulkanAPI>
{
public:
	void initWindow();
	void initVulkan();
	void draw();
	void cleanup();
	GLFWwindow* pWindow;
	glm::mat4* cameraTransform = nullptr;
	glm::vec3 cameraOffset;
	void flushCommandBuffer();

	std::unordered_map<MeshData*, DynamicUBO> uboDynamic;
private:
	friend class Singleton< VulkanAPI>;
	VulkanAPI(){}
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	// single memory allocate
	std::vector<VkBuffer> vertexIndexBuffers;
	std::vector<VkDeviceMemory> vertexIndexBufferMemorys;

	VkBuffer instanceBuffer;
	VkDeviceMemory instanceBufferMemory;
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	//std::vector<VkBuffer> dynamicUniformBuffers;
	//std::vector<VkDeviceMemory> dynamicUniformBuffersMemory;
	std::vector<void*> dynamicUniformData;

	size_t dynamicAlignment;
	size_t normalUBOAlignment;
	//std::vector<VkPushConstantRange> pushContantRange;

	VkRenderPass renderPass;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;

	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	size_t currentFrame = 0;

	//std::vector<Vertex> vertices;
	//std::vector<uint32_t> indices;
	std::vector<InstanceData> instanceDatas;

	uint32_t mipLevels;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	bool frameBufferResized = false;

	void createInstance();
	void setupDebugMessenger();
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	void createDescriptorSetLayout();
	void createGraphicsPipeline();
	void createCommandPool();
	void createColorResource();
	void createDepthResource();
	void createFramebuffers();
	void createTextureImage();
	void createTextureImageView();
	void createTextureSampler();
	void createPushConstants();
	void createVertexIndexBuffer();
	void createInstanceBuffer();
	void createDynamicModels();
	void createUniformBuffers();
	void createDescriptorPool();
	void createDescriptorSet();
	void createCommandBuffers();
	void createSyncObjects();
	void recreateSwapChain();
	void cleanupSwapChain();

	void createInstanceData();
	void updateDynamicUniformBuffer(uint32_t currentImage);
	void updateUniformBuffer(uint32_t currentImage);
	void drawFrame();

	// help function
	static void frameBufferResizeCallback(GLFWwindow* window, int width, int height);
	static std::vector<char> readFile(const std::string& filename);
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	uint32_t findMemoryType(uint32_t typerFilter, VkMemoryPropertyFlags properties);
	bool checkValidationLayerSupport();
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
	VkSampleCountFlagBits getMaxUsableSampleCount();
	std::vector<const char*> getRequiredExtensions();
	bool isDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat findDepthFormat();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	bool hasStencilComponent(VkFormat format);
	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
};