#include "algorithms/move_optimizer.h"
#include "algorithms/lz77_compressor.h"
#include "algorithms/huffman_encoder.h"
#include "core/plugin_manager.h"

namespace mrn {

// 自动注册宏的实现
#define REGISTER_ALGORITHM(ClassName) \
    namespace { \
        struct ClassName##Registrar { \
            ClassName##Registrar() { \
                PluginManager::getInstance().registerAlgorithm( \
                    ClassName::getStaticName(), \
                    std::make_unique<ClassName>() \
                ); \
            } \
        }; \
        static ClassName##Registrar ClassName##_registrar; \
    }

// 注册MoveRun算法
REGISTER_ALGORITHM(MoveRunCompressor)

// 注册LZ77算法  
REGISTER_ALGORITHM(LZ77CompressorPlugin)

// 注册霍夫曼编码算法
REGISTER_ALGORITHM(HuffmanEncoderPlugin)

} // namespace mrn
