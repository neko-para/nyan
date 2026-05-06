#include "ramfs.hpp"

#include "../mount.hpp"

namespace nyan::fs {

RamFSVNode::RamFSVNode(VNodeType type, lib::Ref<SuperBlock> sb, uint32_t mode) {
    __type = type;
    __super_block = sb;
    __mode = mode;

    __inode = ++super_block()->__counter;
}

std::vector<RamFSDirectoryVNode::Entry>::iterator RamFSDirectoryVNode::__find(std::string_view name) noexcept {
    return std::find_if(__entries.begin(), __entries.end(), [&](const Entry& entry) { return entry.__name == name; });
}

lib::Ref<VNode> RamFSDirectoryVNode::lookup(std::string_view name) noexcept {
    if (auto it = __find(name); it != __entries.end()) {
        return it->__vnode;
    } else {
        return {};
    }
}

int RamFSDirectoryVNode::readdir(dirent* buf, size_t size, off_t* offset) noexcept {
    auto* ptr = reinterpret_cast<uint8_t*>(buf);
    size_t written = 0;
    off_t idx = *offset;

    while (idx < (off_t)__entries.size()) {
        const auto& [name, vnode] = __entries[idx];

        size_t reclen = offsetof(struct dirent, d_name) + name.size() + 1;
        reclen = (reclen + 7) & (~7);

        if (written + reclen > size)
            break;

        auto* dent = reinterpret_cast<dirent*>(ptr + written);
        dent->d_ino = vnode->__inode;
        dent->d_off = idx + 1;
        dent->d_reclen = reclen;
        dent->d_type = vnode->__type;
        memcpy(dent->d_name, name.data(), name.size());
        dent->d_name[name.size()] = '\0';

        written += reclen;
        idx++;
    }

    *offset = idx;

    return written;
}

int RamFSDirectoryVNode::mkdir(std::string_view name, uint32_t mode) noexcept {
    if (lookup(name)) {
        return -SYS_EEXIST;
    }
    __entries.push_back({
        name,
        lib::makeRef<RamFSDirectoryVNode>(__super_block, mode),
    });
    return 0;
}

int RamFSDirectoryVNode::create(std::string_view name, uint32_t mode) noexcept {
    if (lookup(name)) {
        return -SYS_EEXIST;
    }
    __entries.push_back({
        name,
        lib::makeRef<RamFSFileVNode>(__super_block, mode),
    });
    return 0;
}

int RamFSDirectoryVNode::link(std::string_view name, lib::Ref<VNode> target) noexcept {
    if (target->__super_block->__fs != __super_block->__fs) {
        return -SYS_EXDEV;
    }
    if (lookup(name)) {
        return -SYS_EEXIST;
    }
    __entries.push_back({
        name,
        target,
    });
    return 0;
}

int RamFSDirectoryVNode::unlink(std::string_view name) noexcept {
    if (auto it = __find(name); it != __entries.end()) {
        __entries.erase(it);
        return 0;
    } else {
        return -SYS_ENOENT;
    }
}

int RamFSDirectoryVNode::stat(struct stat* buf) noexcept {
    memset(buf, 0, sizeof(struct stat));
    buf->st_dev = 1;
    buf->st_mode = S_IFDIR | __mode;
    buf->st_nlink = __ref_count;
    buf->st_size = 0;
    buf->st_blksize = 4096;
    buf->st_ino = __inode;
    return 0;
}

ssize_t RamFSFileVNode::read(void* buf, size_t size, off_t offset) noexcept {
    if (offset < 0) {
        return -SYS_EINVAL;
    }
    if (offset >= __data.size()) {
        return 0;
    }
    auto eff_size = std::min<size_t>(__data.size() - offset, size);
    memcpy(buf, __data.data() + offset, eff_size);
    return eff_size;
}

ssize_t RamFSFileVNode::write(const void* buf, size_t size, off_t offset) noexcept {
    auto newSize = size + offset;
    if (__data.size() <= newSize) {
        __data.resize(newSize, 0);
    }
    memcpy(__data.data() + offset, buf, size);
    return size;
}

int RamFSFileVNode::truncate(off_t length) noexcept {
    if (length < 0) {
        length = 0;
    }
    if (__data.size() > length) {
        __data.resize(length);
    }
    return 0;
}

int RamFSFileVNode::stat(struct stat* buf) noexcept {
    memset(buf, 0, sizeof(struct stat));
    buf->st_dev = 1;
    buf->st_mode = S_IFREG | __mode;
    buf->st_nlink = __ref_count;
    buf->st_size = __data.size();
    buf->st_blksize = 4096;
    buf->st_ino = __inode;
    return 0;
}

ssize_t RamFSCharDevVNode::read(void* buf, size_t size, off_t) noexcept {
    return __device->read(buf, size);
}

ssize_t RamFSCharDevVNode::write(const void* buf, size_t size, off_t) noexcept {
    return __device->write(buf, size);
}

int RamFSCharDevVNode::stat(struct stat* buf) noexcept {
    memset(buf, 0, sizeof(struct stat));
    buf->st_dev = 1;
    buf->st_mode = S_IFCHR | __mode;
    buf->st_nlink = __ref_count;
    buf->st_size = 0;
    buf->st_blksize = 4096;
    buf->st_ino = __inode;
    return 0;
}

int RamFSCharDevVNode::ioctl(uint32_t req, uint32_t param) noexcept {
    return __device->ioctl(req, param);
}

lib::Ref<MountEntry> RamFS::mount(Device*, const char*) noexcept {
    auto entry = lib::makeRef<MountEntry>();
    entry->__super_block = lib::makeRef<RamFSSuperBlock>();
    entry->__super_block->__fs = this;
    entry->__root_node = lib::makeRef<RamFSDirectoryVNode>(entry->__super_block, 0755);
    return entry;
}

}  // namespace nyan::fs
