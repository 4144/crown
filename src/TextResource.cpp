#include "TextResource.h"
#include "FileStream.h"
#include "ResourceArchive.h"
#include "Log.h"
#include "Allocator.h"

namespace crown
{

//-----------------------------------------------------------------------------
void* TextResource::load(Allocator& allocator, ResourceArchive& archive, ResourceId id)
{
	FileStream* stream = archive.open(id);

	CE_ASSERT(stream != NULL, "Resource does not exist: %.8X%.8X", id.name, id.type);

	TextResource* resource = (TextResource*)allocator.allocate(sizeof(TextResource));

	stream->read(&resource->length, sizeof(uint32_t));
	
	resource->data = (char*)allocator.allocate(sizeof(char) * (resource->length + 1));

	stream->read(resource->data, (size_t)resource->length);
	
	resource->data[resource->length] = '\0';

	archive.close(stream);

	return resource;
}

//-----------------------------------------------------------------------------
void TextResource::unload(Allocator& allocator, void* resource)
{
	CE_ASSERT(resource != NULL, "Resource not loaded");

	((TextResource*)resource)->length = 0;

	allocator.deallocate(((TextResource*)resource)->data);
	allocator.deallocate(resource);
}

} // namespace crown
