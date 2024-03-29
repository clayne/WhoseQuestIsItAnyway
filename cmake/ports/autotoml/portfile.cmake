vcpkg_from_github(
	OUT_SOURCE_PATH SOURCE_PATH
	REPO Ryan-rsm-McKenzie/AutoTOML
	REF 10db32f275479a5af15793358e9e9e84079d13b3
	SHA512 a63d1a378eb326f3721de99e404ceaf2f18dda4c6fa41acdf97875be1e3ada7748346fa9f9d40a27b40954495a9d592aabb1216f60506303ec2abd7b90f417df
	HEAD_REF master
)

vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH "lib/cmake/AutoTOML")

file(REMOVE_RECURSE
	"${CURRENT_PACKAGES_DIR}/debug"
	"${CURRENT_PACKAGES_DIR}/lib"
)

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
