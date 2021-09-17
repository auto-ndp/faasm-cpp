set(FAASM_USER darknet)

function(darknet_func exec_name dir_path)
    faasm_func(${exec_name} ${dir_path})
    set(ALL_DARKNET_FUNCS ${ALL_DARKNET_FUNCS} ${exec_name} PARENT_SCOPE)
endfunction(darknet_func)

function(darknet_func_c exec_name dir_path)
    faasm_func_c(${exec_name} ${dir_path})
    set(ALL_DARKNET_FUNCS ${ALL_DARKNET_FUNCS} ${exec_name} PARENT_SCOPE)
endfunction(darknet_func_c)

set(DARKNET_SRCS src/gemm.c src/utils.c src/cuda.c src/deconvolutional_layer.c src/convolutional_layer.c src/list.c
    src/image.c src/activations.c src/im2col.c src/col2im.c src/blas.c src/crop_layer.c src/dropout_layer.c src/maxpool_layer.c
    src/softmax_layer.c src/data.c src/matrix.c src/network.c src/connected_layer.c src/cost_layer.c src/parser.c src/option_list.c
    src/detection_layer.c src/route_layer.c src/upsample_layer.c src/box.c src/normalization_layer.c src/avgpool_layer.c
    src/layer.c src/local_layer.c src/shortcut_layer.c src/logistic_layer.c src/activation_layer.c src/rnn_layer.c
    src/gru_layer.c src/crnn_layer.c src/demo.c src/batchnorm_layer.c src/region_layer.c src/reorg_layer.c src/tree.c
    src/lstm_layer.c src/l2norm_layer.c src/yolo_layer.c src/iseg_layer.c src/image_opencv.cpp

    examples/captcha.c examples/lsd.c examples/super.c examples/art.c examples/tag.c examples/cifar.c examples/go.c examples/rnn.c
    examples/segmenter.c examples/regressor.c examples/classifier.c examples/coco.c examples/yolo.c examples/detector.c
    examples/nightmare.c examples/instance-segmenter.c)
set(DARKNET_MAIN_SRC examples/darknet.c)


darknet_func(darknet ${DARKNET_MAIN_SRC})
target_sources(darknet PRIVATE ${DARKNET_SRCS})
target_include_directories(darknet PRIVATE src/ include/ ../ndp/api)

# Custom target to group all the darknet functions
add_custom_target(darknet_all_funcs DEPENDS ${ALL_DARKNET_FUNCS})
