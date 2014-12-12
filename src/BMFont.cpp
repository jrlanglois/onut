#include "BMFont.h"
#include "SpriteBatch.h"
#include "StringUtils.h"
#include "Texture.h"
#include <sstream>
#include <fstream>

namespace onut {
	int BMFont::parseInt(const std::string& arg, const std::vector<std::string>& lineSplit)	{
		std::stringstream ss;
		ss << BMFont::parseString(arg, lineSplit);
		int theInt = 0;
		if (!(ss >> theInt)) return 0;
		return theInt;
	}

	std::string BMFont::parseString(const std::string& arg, const std::vector<std::string>& lineSplit){
		for (unsigned int i = 1; i < lineSplit.size(); ++i) {
			std::vector<std::string> argSplit = splitString(lineSplit[i], '=');
			if (argSplit[0] == arg) {
				std::string result = argSplit[1];
				if (result[0] == '\"') result = result.substr(1, result.size() - 2);
				return result;
			}
		}
		return "";
	}

	std::shared_ptr<BMFont> BMFont::createFromFile(const std::string& filename) {
		std::ifstream in(filename);
		assert(!in.fail());

		auto pFont = std::make_shared<BMFont>();

		std::string path = filename.substr(0, filename.find_last_of('/') + 1);
		std::string line;
		std::getline(in, line);
		std::vector<std::string> split;
		while (!in.eof()) {
			split = splitString(line, ' ');
			std::string command = split[0];
			if (command == "info") {}
			else if (command == "common") {
				pFont->m_common.lineHeight = parseInt("lineHeight", split);
				pFont->m_common.base = parseInt("base", split);
				pFont->m_common.scaleW = parseInt("scaleW", split);
				pFont->m_common.scaleH = parseInt("scaleH", split);
				pFont->m_common.pages = parseInt("pages", split);
				pFont->m_common.packed = parseInt("packed", split);

				pFont->m_pages = new fntPage*[pFont->m_common.pages];
			}
			else if (command == "page") {
				auto pNewPage = new fntPage();

				pNewPage->id = parseInt("id", split);
				pNewPage->file = parseString("file", split);

				pFont->m_pages[pNewPage->id] = pNewPage;

				// Load its texture
				const auto& texFilename = path + pNewPage->file;
				pNewPage->pTexture = Texture::createFromFile(texFilename);
			}
			else if (command == "chars") {
				pFont->m_charsCount = parseInt("count", split);
			}
			else if (command == "char") {
				fntChar* pNewChar = new fntChar();

				pNewChar->id = parseInt("id", split);
				pNewChar->x = parseInt("x", split);
				pNewChar->y = parseInt("y", split);
				pNewChar->width = parseInt("width", split);
				pNewChar->height = parseInt("height", split);
				pNewChar->xoffset = parseInt("xoffset", split);
				pNewChar->yoffset = parseInt("yoffset", split);
				pNewChar->xadvance = parseInt("xadvance", split);
				pNewChar->page = parseInt("page", split);
				pNewChar->chnl = parseInt("chnl", split);

				pFont->m_chars[pNewChar->id] = pNewChar;
			}

			getline(in, line);
		}
		in.close();

		return pFont;
	}

	BMFont::BMFont() {
	}

	BMFont::~BMFont() {
		for (int i = 0; i < m_common.pages; ++i) {
			delete m_pages[i];
		}
		delete[] m_pages;
		m_pages = nullptr;

		for (auto& kv : m_chars) {
			delete kv.second;
		}
	}

	Vector2 BMFont::measure(const std::string& in_text)	{
		Vector2 result;

		result.y += (float)m_common.lineHeight;
		float curX = 0;
		unsigned int len = in_text.length();
		int charId;
		for (unsigned int i = 0; i < len; ++i) {
			charId = in_text[i];
			if (charId == '\n') {
				result.y += (float)m_common.lineHeight;
				if (curX > result.x) result.x = curX;
				curX = 0;
				continue;
			}
			if (charId == '^' && i + 3 < len) {
				i += 3;
				continue;
			}
			const auto& it = m_chars.find(charId);
			if (it == m_chars.end()) continue;
			auto pDatChar = it->second;
			if (i == len - 1) {
				curX += static_cast<float>(pDatChar->xoffset) + static_cast<float>(pDatChar->width);
			}
			else if (in_text[i + 1] == '\n') {
				curX += static_cast<float>(pDatChar->xoffset) + static_cast<float>(pDatChar->width);
			}
			else {
				curX += static_cast<float>(pDatChar->xadvance);
			}
		}
		if (curX > result.x) result.x = curX;
		
		return result;
	}

	void BMFont::drawInternal(const std::string& text, const Vector2& in_pos, const Color& color, std::shared_ptr<onut::SpriteBatch> pSpriteBatch, Align align) {
		Vector2 pos = in_pos;
		Vector2 dim;
		if (align != Align::TOP_LEFT) {
			dim = measure(text);
			switch (align) {
			case Align::TOP:
				pos.x -= dim.x * .5f;
				break;
			case Align::TOP_RIGHT:
				pos.x -= dim.x;
				break;
			case Align::LEFT:
				pos.y -= dim.y * .5f;
				break;
			case Align::CENTER:
				pos.x -= dim.x * .5f;
				pos.y -= dim.y * .5f;
				break;
			case Align::RIGHT:
				pos.x -= dim.x;
				pos.y -= dim.y * .5f;
				break;
			case Align::BOTTOM_LEFT:
				pos.y -= dim.y;
				break;
			case Align::BOTTOM:
				pos.x -= dim.x * .5f;
				pos.y -= dim.y;
				break;
			case Align::BOTTOM_RIGHT:
				pos.x -= dim.x;
				pos.y -= dim.y;
				break;
			}
		}
		Vector2 curPos = { static_cast<float>(static_cast<int>(pos.x)), static_cast<float>(static_cast<int>(pos.y)) };
		unsigned int len = text.length();
		int page = -1;
		float r, g, b;
		unsigned int i = 0;
		Color curColor = color;
		pSpriteBatch->begin();
		for (const auto& charId : text) {
			if (charId == '\n')	{
				curPos.x = pos.x;
				curPos.y += static_cast<float>(m_common.lineHeight);
				++i;
				continue;
			}
			if (charId == '^' && i + 3 < len){
				// Colored text!
				r = (static_cast<float>(text[i + 1]) - static_cast<float>('0')) / 9.0f;
				g = (static_cast<float>(text[i + 2]) - static_cast<float>('0')) / 9.0f;
				b = (static_cast<float>(text[i + 3]) - static_cast<float>('0')) / 9.0f;
				curColor = { r, g, b, color.w };
				i += 4;
				continue;
			}
			if (!m_chars.count(charId)) continue;
			auto pDatChar = m_chars[charId];
			auto pTexture = m_pages[pDatChar->page]->pTexture;

			// Draw it here
			pSpriteBatch->drawRectWithUVs(
				pTexture, {
					curPos.x + static_cast<float>(pDatChar->xoffset), curPos.y + static_cast<float>(pDatChar->yoffset),
					static_cast<float>(pDatChar->width), static_cast<float>(pDatChar->height)
				}, {
					static_cast<float>(pDatChar->x) / static_cast<float>(m_common.scaleW),
					static_cast<float>(pDatChar->y) / static_cast<float>(m_common.scaleH),
					static_cast<float>(pDatChar->x + pDatChar->width) / static_cast<float>(m_common.scaleW),
					static_cast<float>(pDatChar->y + pDatChar->height) / static_cast<float>(m_common.scaleH)
				}, curColor);

			curPos.x += static_cast<float>(pDatChar->xadvance);
			++i;
		}
		pSpriteBatch->end();
	}
}
