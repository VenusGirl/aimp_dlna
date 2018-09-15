#pragma once

using namespace std;

class AimpDlnaDataProviderSelection : public IUnknownInterfaceImpl<IAIMPMLDataProviderSelection, IID_IAIMPMLDataProviderSelection> {
private:
	string nodeId;
	unsigned int index;
	PLT_MediaObjectListReference items;
	vector<wstring> fields;

	PLT_MediaObject* GetItem(unsigned int index);
	wstring GetField(unsigned int index);

public:
	AimpDlnaDataProviderSelection(string nodeId, PLT_MediaObjectListReference items, vector<wstring> fields) {
		this->nodeId = nodeId;
		this->items = items;
		this->fields = fields;
		this->index = 0;
	}

	~AimpDlnaDataProviderSelection() {
		items.Detach();
	}

	DOUBLE WINAPI GetValueAsFloat(int FieldIndex);
	int WINAPI GetValueAsInt32(int FieldIndex);
	INT64 WINAPI GetValueAsInt64(int FieldIndex) { return INT64(); }
	WCHAR* WINAPI GetValueAsString(int FieldIndex, int* Length);

	BOOL WINAPI HasNextPage() { return false; }
	BOOL WINAPI NextRow();
};

