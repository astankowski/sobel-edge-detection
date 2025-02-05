#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>

struct BITMAPFILEHEADER {
  uint16_t bfType{};
  uint32_t bfSize{};
  uint16_t bfReserved1{};
  uint16_t bfReserved2{};
  uint32_t bfOffBits{};  
};

struct BITMAPINFOHEADER {
  uint32_t biSize{};
  uint32_t biWidth{};
  uint32_t biHeight{};
  uint16_t biPlanes{};
  uint16_t biBitCount{};
  uint32_t biCompression{};
  uint32_t biSizeImage{};
  uint32_t biXpelsPerMeter{};
  uint32_t biYpelsPerMeter{};
  uint32_t biCrlUses{};
  uint32_t biCrlImportant{};
};

struct Pixel {
  uint8_t blue{};
  uint8_t red{};
  uint8_t green{};
};

class BMP {
public:
  void read(const std::string filename);
  void write(const std::string filename) const;
  void applySobelFilter();
  void showProperties() const;

private:
  BITMAPFILEHEADER fileHeader;
  BITMAPINFOHEADER infoHeader;
  std::vector<Pixel> pixels;

  static constexpr int sobelKernels[8][9] = {
    {-1, 0, 1, -2, 0, 2, -1, 0, 1},
    {0, 1, 2, -1, 0, 1, -2, -1, 0},
    {1, 2, 1, 0, 0, 0, -1, -2, -1},
    {2, 1, 0, 1, 0, -1, 0, -1, -2},
    {1, 0, -1, 2, 0, -2, 1, 0, -1},
    {0, -1, -2, 1, 0, -1, 2, 1, 0},
    {-1, -2, -1, 0, 0, 0, 1, 2, 1},
    {-2, -1, 0, -1, 0, 1, 0, 1, 2}
  };
};

void BMP::read(const std::string filename) {
  std::ifstream file{filename, std::ios::binary};
  if (!file) {
    throw std::runtime_error("Failed to open file: " + filename);
  }

  file.read((char*)&fileHeader.bfType, 2);
  file.read((char*)&fileHeader.bfSize, 4);
  file.read((char*)&fileHeader.bfReserved1, 2);
  file.read((char*)&fileHeader.bfReserved2, 2);
  file.read((char*)&fileHeader.bfOffBits, 4);

  file.read((char*)&infoHeader.biSize, 4);
  file.read((char*)&infoHeader.biWidth, 4);
  file.read((char*)&infoHeader.biHeight, 4);
  file.read((char*)&infoHeader.biPlanes, 2);
  file.read((char*)&infoHeader.biBitCount, 2);
  file.read((char*)&infoHeader.biCompression, 4);
  file.read((char*)&infoHeader.biSizeImage, 4);
  file.read((char*)&infoHeader.biXpelsPerMeter, 4);
  file.read((char*)&infoHeader.biYpelsPerMeter, 4);
  file.read((char*)&infoHeader.biCrlUses, 4);
  file.read((char*)&infoHeader.biCrlImportant, 4);
  
  pixels.resize(infoHeader.biWidth * infoHeader.biHeight);
  int padding = (infoHeader.biWidth * 3) % 4;
  
  for (unsigned int i = 0; i < pixels.size(); i++) {
    file.read(reinterpret_cast<char*>(&pixels[i]), sizeof(Pixel));
    if ((i + 1) % infoHeader.biWidth == 0) {
      file.ignore(padding);
    }
  }
  
  file.close();
}

void BMP::applySobelFilter() {
  std::vector<Pixel> pixelsCopy = pixels;

  for (unsigned int y = 1; y < infoHeader.biHeight - 1; ++y) {
    for (unsigned int x = 1; x < infoHeader.biWidth - 1; ++x) {
      double avgBlue = 0, avgRed = 0, avgGreen = 0;
      
      for (const auto& kernel : sobelKernels) {
        double b = 0, r = 0, g = 0;
        
        for (int ky = -1; ky <= 1; ++ky) {
          for (int kx = -1; kx <= 1; ++kx) {
            int idx = (y + ky) * infoHeader.biWidth + (x + kx);
            int kernelIdx = (ky + 1) * 3 + (kx + 1);
            
            b += pixelsCopy[idx].blue * kernel[kernelIdx];
            r += pixelsCopy[idx].red * kernel[kernelIdx];
            g += pixelsCopy[idx].green * kernel[kernelIdx];
          }
        }
        avgBlue += std::abs(b);
        avgRed += std::abs(r);
        avgGreen += std::abs(g);
      }
      
      avgBlue /= 8;
      avgRed /= 8;
      avgGreen /= 8;
      int minThreshold = 33;
      
      int idx = y * infoHeader.biWidth + x;
      pixels[idx].blue = std::clamp(static_cast<int>(avgBlue), minThreshold, 255);
      pixels[idx].red = std::clamp(static_cast<int>(avgRed), minThreshold, 255);
      pixels[idx].green = std::clamp(static_cast<int>(avgGreen), minThreshold, 255);
    }
  }
}

void BMP::showProperties() const {
  std::cout
    << "\nNaglowek pliku opisujacy format pliku: "
    << "\n\tSygnatura pliku: " << fileHeader.bfType
    << "\n\tDlugosc calego pliku w bajtach: " << fileHeader.bfSize
    << "\n\tPole zarezerwowane 1: " << fileHeader.bfReserved1
    << "\n\tPole zarezerwowane 2: " << fileHeader.bfReserved1
    << "\n\tPozycja danych obrazowych w pliku: " << fileHeader.bfOffBits
    << "\n\nNaglowek obrazu: "
    << "\n\tRozmiar naglowka informacyjnego: " << infoHeader.biSize
    << "\n\tSzerokosc obrazu w pikselach: " << infoHeader.biWidth
    << "\n\tWysokosc obrazu w pikselach: " << infoHeader.biHeight
    << "\n\tLiczba platow: " << infoHeader.biPlanes
    << "\n\tLiczba bitow na piksel: " << infoHeader.biBitCount
    << "\n\tAlgorytm kompresji: " << infoHeader.biCompression
    << "\n\tRozmiar rysunku: " << infoHeader.biSizeImage
    << "\n\tRozdzielczosc pozioma: " << infoHeader.biXpelsPerMeter
    << "\n\tRozdzielczosc pionowa: " << infoHeader.biYpelsPerMeter
    << "\n\tLiczba kolorow w palecie: " << infoHeader.biCrlUses
    << "\n\tLiczba waznych kolorow w palecie: " << infoHeader.biCrlImportant
    << std::endl;
};

void BMP::write(const std::string filename) const
{
  std::ofstream file{filename, std::ios::binary };
  if (!file) {
    throw std::runtime_error("Failed to open file: " + filename);
  }

  file.write((char*)&fileHeader.bfType, 2);
  file.write((char*)&fileHeader.bfSize, 4);
  file.write((char*)&fileHeader.bfReserved1, 2);
  file.write((char*)&fileHeader.bfReserved2, 2);
  file.write((char*)&fileHeader.bfOffBits, 4);

  file.write((char*)&infoHeader.biSize, 4);
  file.write((char*)&infoHeader.biWidth, 4);
  file.write((char*)&infoHeader.biHeight, 4);
  file.write((char*)&infoHeader.biPlanes, 2);
  file.write((char*)&infoHeader.biBitCount, 2);
  file.write((char*)&infoHeader.biCompression, 4);
  file.write((char*)&infoHeader.biSizeImage, 4);
  file.write((char*)&infoHeader.biXpelsPerMeter, 4);
  file.write((char*)&infoHeader.biYpelsPerMeter, 4);
  file.write((char*)&infoHeader.biCrlUses, 4);
  file.write((char*)&infoHeader.biCrlImportant, 4);
  
  int padding = (infoHeader.biWidth * 3) % 4;
  uint8_t pad = 0;

  for (unsigned int i = 0; i < infoHeader.biWidth * infoHeader.biHeight; i++) {
    file << pixels[i].blue << pixels[i].red << pixels[i].green;
    
    if (padding && (i + 1) % infoHeader.biWidth == 0) {
      for (int j = 0; j < 4 - padding; j++) {
        file << pad;
      }
    }
  }

  file.close();
}

int main()
{
  BMP bitmap;
  std::vector<Pixel> pixelsCopy;
  std::string filename;
  
  std::cout << "Padaj sciezke pliku wejsciowego: ";
  std::cin >> filename;
  bitmap.read(filename);
  
  bitmap.showProperties();
  bitmap.applySobelFilter();

  std::cout << "Padaj sciezke pliku wyjsciowego: ";
  std::cin >> filename;
  bitmap.write(filename);

  return 0;
}
