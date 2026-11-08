#include <cstring>
#include <stdexcept>
#include <limits>
#include "memlist.h"
CMemList::CMemList() : m_root{} {}
CMemList::~CMemList() { Clear(); }
void CMemList::Clear() {
    ml_item* p = m_root.top;
    while (p) { ml_item* next = p->nxt; delete[] reinterpret_cast<char*>(p); p = next; }
    m_root = ml_root{};
}
void* CMemList::Add(void* data, long n, void** member) {
    if (n < 0 || n > std::numeric_limits<int>::max() || (!data && n)) throw std::invalid_argument("Invalid memory list data");
    ml_item* p = reinterpret_cast<ml_item*>(new char[sizeof(ml_item) + static_cast<size_t>(n)]{});
    p->siz = static_cast<int>(n); p->prv = m_root.las;
    if (n) std::memcpy(p->mbr, data, n);
    if (m_root.las) m_root.las->nxt = p; else m_root.top = p;
    m_root.las = p; ++m_root.cnt; if (member) *member = p->mbr; return p;
}
int CMemList::Delete(void* handle) {
    ml_item* p = m_root.top;
    while (p && p != handle) p = p->nxt;
    if (!p) return -1;
    if (p->prv) p->prv->nxt = p->nxt; else m_root.top = p->nxt;
    if (p->nxt) p->nxt->prv = p->prv; else m_root.las = p->prv;
    delete[] reinterpret_cast<char*>(p); --m_root.cnt; return 0;
}
void* CMemList::Make() { return nullptr; } // Unspecified legacy operation.
void* CMemList::List() { return m_root.top; }
int CMemList::Count() { return m_root.cnt; }
void* CMemList::Top(void** member) { if (member) *member = m_root.top ? m_root.top->mbr : nullptr; return m_root.top; }
void* CMemList::Next(void* h, void** member) { ml_item* p = h ? static_cast<ml_item*>(h)->nxt : nullptr; if (member) *member = p ? p->mbr : nullptr; return p; }
void* CMemList::Member(void* h) { return h ? static_cast<ml_item*>(h)->mbr : nullptr; }
