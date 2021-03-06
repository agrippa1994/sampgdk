/* Copyright (C) 2012-2014 Zeex
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

#include <sampgdk/platform.h>

#if SAMPGDK_WINDOWS
  #include <windows.h>
#else
  #include <dlfcn.h>
  #include <string.h>
#endif

#include "plugin.h"

static struct sampgdk_plugin_list *_sampgdk_plugins;

int sampgdk_plugin_register(void *plugin) {
  struct sampgdk_plugin_list *ptr;

  assert(plugin != NULL);

  ptr = malloc(sizeof(*ptr));
  if (ptr == NULL) {
    return -ENOMEM;
  }

  ptr->plugin = plugin;
  ptr->next = _sampgdk_plugins;
  _sampgdk_plugins = ptr;

  return 0;
}

int sampgdk_plugin_unregister(void *plugin) {
  struct sampgdk_plugin_list *prev;
  struct sampgdk_plugin_list *cur;

  assert(plugin != NULL);

  cur = _sampgdk_plugins;
  prev = NULL;

  while (cur != NULL) {
    if (cur->plugin != plugin) {
      continue;
    }

    if (prev != NULL) {
      prev->next = cur->next;
    } else {
      assert(_sampgdk_plugins == cur);
      _sampgdk_plugins = cur->next;
    }

    free(cur);
    return 0;
  }

  return -EINVAL;
}

bool sampgdk_plugin_is_registered(void *plugin) {
  struct sampgdk_plugin_list *cur;

  assert(plugin != NULL);

  cur = _sampgdk_plugins;

  while (cur != NULL) {
    if (cur->plugin == plugin) {
      return true;
    }
    cur = cur->next;
  }

  return false;
}

struct sampgdk_plugin_list *sampgdk_plugin_get_list(void) {
  return _sampgdk_plugins;
}

#if SAMPGDK_WINDOWS

void *sampgdk_plugin_get_symbol(void *plugin, const char *name)  {
  assert(plugin != NULL);
  assert(name != NULL);
  return (void*)GetProcAddress((HMODULE)plugin, name);
}

void *sampgdk_plugin_get_handle(void *address) {
  MEMORY_BASIC_INFORMATION mbi;
  assert(address != NULL);
  if (VirtualQuery(address, &mbi, sizeof(mbi)) == 0) {
    return NULL;
  }
  return (void*)mbi.AllocationBase;
}

void sampgdk_plugin_get_filename(void *address, char *filename, size_t size) {
  HMODULE module = (HMODULE)sampgdk_plugin_get_handle(address);
  assert(address != NULL);
  assert(filename != NULL);
  GetModuleFileName(module, filename, size);
}

#else /* SAMPGDK_WINDOWS */

void *sampgdk_plugin_get_symbol(void *plugin, const char *name)  {
  assert(plugin != NULL);
  assert(name != NULL);
  return dlsym(plugin, name);
}

void *sampgdk_plugin_get_handle(void *address) {
  Dl_info info;
  assert(address != NULL);
  if (dladdr(address, &info) != 0) {
    return dlopen(info.dli_fname, RTLD_NOW);
  }
  return NULL;
}

void sampgdk_plugin_get_filename(void *address, char *filename, size_t size) {
  Dl_info info;
  assert(address != NULL);
  assert(filename != NULL);
  if (dladdr(address, &info) != 0) {
    strncpy(filename, info.dli_fname, size);
  }
}

#endif /* !SAMPGDK_WINDOWS */
