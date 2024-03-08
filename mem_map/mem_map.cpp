#include "H5Cpp.h"
#include "hdf5_hl.h"
#include <stdexcept>
#include <vector>
#include <fstream>

void
file_to_buffer(const std::string &filepath, std::vector<char> &buffer)
{
}

void
buffer_to_file(const std::vector<char> &buffer, const std::string &filepath)
{
  std::ofstream stream{filepath};
  stream.write(buffer.data(), buffer.size());
}

void
data_to_h5file(const std::vector<double>& data, H5::H5File& file)
{
    hsize_t dim[1];
    dim[0] = data.size();
    H5::DataSpace space{1, dim};
    auto          type    = H5::PredType::NATIVE_DOUBLE;
    auto          dataset = file.createDataSet("/ds", type, space);
    dataset.write(data.data(), type);
}

void
data_to_buffer(const std::vector<double> &data, std::vector<char> &buffer)
{
    /* create the HDF5 file image first */
    H5::FileAccPropList prop_list = H5::FileAccPropList::DEFAULT;

    herr_t h5err =
        H5Pset_fapl_core(prop_list.getId(), /* memory increment size: 4M */ 1 << 22, /*backing_store*/ false);

    if (h5err < 0)
        throw std::runtime_error("H5P_set_fapl_core failed.");

    H5::H5File file{"mem", H5F_ACC_TRUNC, H5::FileCreatPropList::DEFAULT, prop_list};

    data_to_h5file(data, file);

    /* get the image */
    file.flush(H5F_SCOPE_LOCAL); // probably not necessary
    ssize_t img_size = H5Fget_file_image(file.getId(), NULL, 0);
    buffer.resize(img_size);

    // second call to actually copy the data into our buffer
    H5Fget_file_image(file.getId(), buffer.data(), img_size);
}

void
h5file_to_data(H5::H5File& file, std::vector<double>& data)
{
    auto     dataset = file.openDataSet("/ds");
    auto     dataspace   = dataset.getSpace();
    hsize_t dim[1];
    auto rank = dataspace.getSimpleExtentDims(dim, nullptr);
    H5::DataSpace memspace{rank, dim};
    data.resize(dim[0]);
    dataset.read(data.data(), H5::PredType::NATIVE_DOUBLE, memspace, dataspace);
}

void
buffer_to_data(const std::vector<char> &buffer, std::vector<double> &data)
{
    auto       hid = H5LTopen_file_image((void *)buffer.data(), sizeof(buffer[0]) * buffer.size(),
                                         H5LT_FILE_IMAGE_DONT_RELEASE | H5LT_FILE_IMAGE_DONT_COPY);
    H5::H5File file;
    file.setId(hid);
    h5file_to_data(file, data);
}

void
data_to_file(const std::vector<double> &data, const std::string &filepath)
{
  // TODO This list seems to be global
  // Without explicitly passing the list (with backing_store true) to to H5File constructor,
  // we will have problem when opening file
  H5::FileAccPropList prop_list = H5::FileAccPropList::DEFAULT;

  herr_t h5err =
    H5Pset_fapl_core(prop_list.getId(), /* memory increment size: 4M */ 1 << 22, /*backing_store*/ true);

  if (h5err < 0)
    throw std::runtime_error("H5P_set_fapl_core failed.");

  H5::H5File file{filepath, H5F_ACC_TRUNC, H5::FileCreatPropList::DEFAULT, prop_list};
  data_to_h5file(data, file);
}

void
file_to_data(const std::string &filepath, std::vector<double> &data)
{
  H5::H5File file{filepath, H5F_ACC_RDONLY};
  h5file_to_data(file, data);
}

void
print(const char* title, const std::vector<double>& data)
{
  printf("%-40s: ", title);
  for (auto v : data) {
    printf("%f ", v);
  }
  printf("\n");
}

int
main(int argc, char *argv[])
{
  std::vector<char> buffer;
  {
    const std::vector<double> data = {0.0, 0.1, 1.2, 2.3};
    print("Data", data);
    data_to_buffer(data, buffer);
  }
  {
    std::vector<double> result;
    buffer_to_data(buffer, result);

    print("Data from H5 buffer", result);
  }
  {
    const std::string filepath{"/tmp/test-h5-mem-map.h5"};
    std::remove(filepath.c_str());
    std::vector<double> result;

    buffer_to_file(buffer, filepath);
    file_to_data(filepath, result);

    print("Data from H5 file (from H5 buffer)", result);
  }
  {
    const std::string filepath{"/tmp/test-h5-mem-map.h5"};
    std::remove(filepath.c_str());
    {
      const std::vector<double> data = {0.0, 0.1, 1.2, 2.3};
      data_to_file(data, filepath);
    }
    {
      std::vector<double> result;
      file_to_data(filepath, result);
      print("Data from H5 file (from data)", result);
    }
  }
}
