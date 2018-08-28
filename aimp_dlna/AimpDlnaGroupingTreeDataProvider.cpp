#include "stdafx.h"
#include "AimpDlnaGroupingTreeDataProvider.h"
#include "AimpDlnaGroupingTreeDataProviderSelection.h"

using namespace std;

HRESULT WINAPI AimpDlnaGroupingTreeDataProvider::AppendFilter(IAIMPMLDataFilterGroup* Filter, IAIMPMLGroupingTreeSelection* Selection) {
	Filter->BeginUpdate();
	Filter->SetValueAsInt32(AIMPML_FILTERGROUP_OPERATION, AIMPML_FILTERGROUP_OPERATION_AND);
	for (size_t i = 0; i < Selection->GetCount(); i++) {
		IAIMPString* fieldName = nullptr;
		VARIANT value;

		if (SUCCEEDED(Selection->GetValue(i, &fieldName, &value))) {
			IAIMPMLDataFieldFilter* outFilter = nullptr;
			if (FAILED(Filter->Add(fieldName, &value, &VARIANT(), AIMPML_FIELDFILTER_OPERATION_EQUALS, &outFilter))) {
				return E_FAIL;
			}
		}
	}
	Filter->EndUpdate();
	return S_OK;
}

DWORD WINAPI AimpDlnaGroupingTreeDataProvider::GetCapabilities() {
	return AIMPML_GROUPINGTREEDATAPROVIDER_CAP_HIDEALLDATA | AIMPML_GROUPINGTREEDATAPROVIDER_CAP_DONTSORT;
}

HRESULT WINAPI AimpDlnaGroupingTreeDataProvider::GetData(IAIMPMLGroupingTreeSelection* Selection, IAIMPMLGroupingTreeDataProviderSelection** Data) {
	if (Selection->GetCount() == 0) {
		return GetRootData(Data);
	} else {
		return GetChildrenData(Selection, Data);
	}
}

HRESULT AimpDlnaGroupingTreeDataProvider::GetRootData(IAIMPMLGroupingTreeDataProviderSelection** Data){
	auto list = vector<AimpDlnaGroupingTreeDataProviderNode>();

	const auto& devices = mediaBrowser->GetMediaServers();
	NPT_AutoLock lock((NPT_Mutex&)devices);

	wstring_convert<codecvt_utf8<wchar_t>, wchar_t> converter;
	for (auto device = devices.GetFirstItem(); device; device++) {
		auto displayName = converter.from_bytes((*device)->GetFriendlyName().GetChars());
		auto value = converter.from_bytes((*device)->GetUUID().GetChars());
		AimpDlnaGroupingTreeDataProviderNode node = { AIMPML_FIELDIMAGE_NOTE, value, displayName, true, true };
		list.push_back(node);
	}

	*Data = new AimpDlnaGroupingTreeDataProviderSelection(list);
	return S_OK;
}

HRESULT AimpDlnaGroupingTreeDataProvider::GetChildrenData(IAIMPMLGroupingTreeSelection* Selection, IAIMPMLGroupingTreeDataProviderSelection** Data) {
	vector<wstring> breadcrumbs;

	for (size_t i = 0; i < Selection->GetCount(); i++) {
		IAIMPString* fieldName = nullptr;
		VARIANT value;

		if (SUCCEEDED(Selection->GetValue(i, &fieldName, &value))) {
			breadcrumbs.push_back(wstring(value.bstrVal));
		}
	}

	if (breadcrumbs.size() == 0)
		return E_FAIL;

	wstring_convert<codecvt_utf8<wchar_t>, wchar_t> converter;
	auto deviceUuid = converter.to_bytes(breadcrumbs.back());
	auto containerId = breadcrumbs.size() == 1 ? "0" : converter.to_bytes(breadcrumbs.front());

	PLT_DeviceDataReference device;
	if (FAILED(mediaBrowser->FindServer(deviceUuid.c_str(), device)))
		return E_FAIL;


	PLT_MediaObjectListReference objects;
	if (FAILED(mediaBrowser->BrowseSync(device, containerId.c_str(), objects, false, 0, UINT_MAX)))
		return E_FAIL;

	auto list = vector<AimpDlnaGroupingTreeDataProviderNode>();
	for (auto object = objects->GetFirstItem(); object; object++) {
		if (!(*object)->IsContainer())
			continue;

		auto displayName = converter.from_bytes((*object)->m_Title.GetChars());
		auto value = converter.from_bytes((*object)->m_ObjectID.GetChars());
		list.push_back({ AIMPML_FIELDIMAGE_FOLDER , value, displayName, false, true });
	}

	*Data = new AimpDlnaGroupingTreeDataProviderSelection(list);
	return S_OK;
}

HRESULT WINAPI AimpDlnaGroupingTreeDataProvider::GetFieldForAlphabeticIndex(IAIMPString** FieldName) { return E_FAIL; }

HRESULT WINAPI AimpDlnaGroupingTreeDataProvider::QueryInterface(REFIID riid, LPVOID* ppvObject) {
	if (riid == IID_IAIMPMLGroupingTreeDataProvider) {
		*ppvObject = this;
		AddRef();
		return S_OK;
	}

	return Base::QueryInterface(riid, ppvObject);
}