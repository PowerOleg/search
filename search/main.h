#pragma once

void UpdateLinks(std::queue<std::string>& links_all, std::vector<std::shared_ptr<Webpage>>& pages, std::atomic_int& pages_count, size_t& ñountdown, int& retFlag);
