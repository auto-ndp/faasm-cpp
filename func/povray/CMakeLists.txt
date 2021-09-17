set(FAASM_USER povray)

faasm_func(povray povray.cpp)
target_sources(povray PRIVATE atmosph.cpp bbox.cpp bcyl.cpp benchmark.cpp bezier.cpp blob.cpp boxes.cpp bsphere.cpp camera.cpp chi2.cpp colour.cpp colutils.cpp
  cones.cpp csg.cpp discs.cpp express.cpp fncode.cpp fnintern.cpp fnpovfpu.cpp fnsyntax.cpp fpmetric.cpp fractal.cpp function.cpp hcmplx.cpp hfield.cpp histogra.cpp
  iff.cpp image.cpp interior.cpp isosurf.cpp lathe.cpp lbuffer.cpp lightgrp.cpp lighting.cpp mathutil.cpp matrices.cpp media.cpp mesh.cpp normal.cpp objects.cpp
  octree.cpp optout.cpp parse.cpp parsestr.cpp parstxtr.cpp pattern.cpp pgm.cpp photons.cpp pigment.cpp planes.cpp point.cpp poly.cpp polygon.cpp polysolv.cpp povmsend.cpp
  povmsrec.cpp povray.cpp pov_mem.cpp pov_util.cpp ppm.cpp prism.cpp quadrics.cpp quatern.cpp radiosit.cpp rad_data.cpp ray.cpp rendctrl.cpp render.cpp renderio.cpp
  reswords.cpp sor.cpp spheres.cpp sphsweep.cpp splines.cpp statspov.cpp super.cpp targa.cpp texture.cpp tokenize.cpp torus.cpp triangle.cpp truetype.cpp txttest.cpp
  userdisp.cpp userio.cpp vbuffer.cpp vlbuffer.cpp warps.cpp

  base/fileinputoutput.cpp base/povms.cpp base/povmscpp.cpp base/processoptions.cpp base/stringutilities.cpp base/textstream.cpp
  base/textstreambuffer.cpp

  frontend/defaultplatformbase.cpp frontend/defaultrenderfrontend.cpp frontend/messageoutput.cpp frontend/processrenderoptions.cpp frontend/renderfrontend.cpp

  spec_qsort/spec_qsort.c)

target_include_directories(povray SYSTEM BEFORE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/frontend"
  "${CMAKE_CURRENT_SOURCE_DIR}/base" "${CMAKE_CURRENT_SOURCE_DIR}/spec_qsort" "${CMAKE_CURRENT_SOURCE_DIR}/../ndp/api")
target_compile_definitions(povray PUBLIC SPEC SPECCPU NDEBUG NOPERFLIB NOREDUCE SPEC_AUTO_SUPPRESS_OPENMP IN_FAASM -Dregister=)
set(ALL_POVRAY_FUNCS povray)

add_custom_target(povray_all_funcs DEPENDS povray)
