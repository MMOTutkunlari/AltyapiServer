#ifndef __CSVFILE_H__
#define __CSVFILE_H__

#include <string>
#include <vector>

#if _MSC_VER
    #include <hash_map>
#else
    #include <map>
#endif

////////////////////////////////////////////////////////////////////////////////
/// \class cCsvAlias
/// \brief CSV ������ �������� �� �߻��ϴ� �ε��� ������ ���̱� ���� 
/// ���� ��ü.
///
/// ���� ��� 0�� �÷��� A�� ���� ������ �����ϰ�, 1�� �÷��� B�� ���� ������ 
/// �����ϰ� �־��µ�...
///
/// <pre>
/// int a = row.AsInt(0);
/// int b = row.AsInt(1);
/// </pre>
///
/// �� ���̿� C�� ���� ������ �����ϴ� �÷��� ����� ���, �ϵ��ڵ��Ǿ� �ִ� 
/// 1���� ã�Ƽ� ���ľ� �ϴµ�, ����� ������ �߻��ϱ� ���� �۾��̴�. 
///
/// <pre>
/// int a = row.AsInt(0);
/// int c = row.AsInt(1);
/// int b = row.AsInt(2); <-- �� �κ��� ������ �Ű��� �Ѵ�.
/// </pre>
/// 
/// �� �κ��� ���ڿ��� ó���ϸ� ���������� ���� ���� �ణ�̳��� ���� �� 
/// �ִ�.
////////////////////////////////////////////////////////////////////////////////

class cCsvAlias
{
private:
#if _MSC_VER
    typedef stdext::hash_map<std::string, size_t> NAME2INDEX_MAP;
    typedef stdext::hash_map<size_t, std::string> INDEX2NAME_MAP;
#else
    typedef std::map<std::string, size_t> NAME2INDEX_MAP;
    typedef std::map<size_t, std::string> INDEX2NAME_MAP;
#endif

    NAME2INDEX_MAP m_Name2Index;  ///< �� �ε��� ������� ����ϱ� ���� �̸���
    INDEX2NAME_MAP m_Index2Name;  ///< �߸��� alias�� �˻��ϱ� ���� �߰����� ��


public:
    /// \brief ������
    cCsvAlias() {} 

    /// \brief �Ҹ���
    virtual ~cCsvAlias() {}


public:
    /// \brief ���� �׼����� ��, ���� ��� ����� �̸��� ����Ѵ�.
    void AddAlias(const char* name, size_t index);

    /// \brief ��� �����͸� �����Ѵ�.
    void Destroy();

    /// \brief ���� �ε����� �̸����� ��ȯ�Ѵ�.
    const char* operator [] (size_t index) const;

    /// \brief �̸��� ���� �ε����� ��ȯ�Ѵ�.
    size_t operator [] (const char* name) const;


private:
    /// \brief ���� ������ ����
    cCsvAlias(const cCsvAlias&) {}

    /// \brief ���� ������ ����
    const cCsvAlias& operator = (const cCsvAlias&) { return *this; }
};


////////////////////////////////////////////////////////////////////////////////
/// \class cCsvRow 
/// \brief CSV ������ �� ���� ĸ��ȭ�� Ŭ����
///
/// CSV�� �⺻ ������ �������� ���̴� �ϳ��� ���� ',' ���ڷ� ������ ���̴�.
/// ������, �� �ȿ� Ư�� ���ڷ� ���̴� ',' ���ڳ� '"' ���ڰ� �� ���, 
/// ����� �ణ �̻��ϰ� ���Ѵ�. ������ �� ��ȭ�� ���̴�.
/// 
/// <pre>
/// �������� ���̴� ��� | ���� CSV ���Ͽ� ���ִ� ���
/// ---------------------+----------------------------------------------------
/// ItemPrice            | ItemPrice
/// Item,Price           | "Item,Price"
/// Item"Price           | "Item""Price"
/// "ItemPrice"          | """ItemPrice"""
/// "Item,Price"         | """Item,Price"""
/// Item",Price          | "Item"",Price"
/// </pre>
/// 
/// �� ���μ� ������ ���� ������ �� �� �ִ�.
/// - �� ���ο� ',' �Ǵ� '"' ���ڰ� �� ���, �� �¿쿡 '"' ���ڰ� �����.
/// - �� ������ '"' ���ڴ� 2���� ġȯ�ȴ�.
///
/// \sa cCsvFile
////////////////////////////////////////////////////////////////////////////////

class cCsvRow : public std::vector<std::string>
{
public:
    /// \brief �⺻ ������
    cCsvRow() {}

    /// \brief �Ҹ���
    ~cCsvRow() {}


public:
    /// \brief �ش� ���� �����͸� int ������ ��ȯ�Ѵ�.
    int AsInt(size_t index) const { return atoi(at(index).c_str()); }

    /// \brief �ش� ���� �����͸� double ������ ��ȯ�Ѵ�.
    double AsDouble(size_t index) const { return atof(at(index).c_str()); }

    /// \brief �ش� ���� �����͸� ���ڿ��� ��ȯ�Ѵ�.
    const char* AsString(size_t index) const { return at(index).c_str(); }

    /// \brief �ش��ϴ� �̸��� �� �����͸� int ������ ��ȯ�Ѵ�.
    int AsInt(const char* name, const cCsvAlias& alias) const {
        return atoi( at(alias[name]).c_str() ); 
    }

    /// \brief �ش��ϴ� �̸��� �� �����͸� int ������ ��ȯ�Ѵ�.
    double AsDouble(const char* name, const cCsvAlias& alias) const {
        return atof( at(alias[name]).c_str() ); 
    }

    /// \brief �ش��ϴ� �̸��� �� �����͸� ���ڿ��� ��ȯ�Ѵ�.
    const char* AsString(const char* name, const cCsvAlias& alias) const { 
        return at(alias[name]).c_str(); 
    }


private:
    /// \brief ���� ������ ����
    cCsvRow(const cCsvRow&) {}

    /// \brief ���� ������ ����
    const cCsvRow& operator = (const cCsvRow&) { return *this; }
};


////////////////////////////////////////////////////////////////////////////////
/// \class cCsvFile
/// \brief CSV(Comma Seperated Values) ������ read/write�ϱ� ���� Ŭ����
///
/// <b>sample</b>
/// <pre>
/// cCsvFile file;
///
/// cCsvRow row1, row2, row3;
/// row1.push_back("ItemPrice");
/// row1.push_back("Item,Price");
/// row1.push_back("Item\"Price");
///
/// row2.reserve(3);
/// row2[0] = "\"ItemPrice\"";
/// row2[1] = "\"Item,Price\"";
/// row2[2] = "Item\",Price\"";
///
/// row3 = "\"ItemPrice\"\"Item,Price\"Item\",Price\"";
///
/// file.add(row1);
/// file.add(row2);
/// file.add(row3);
/// file.save("test.csv", false);
/// </pre>
///
/// \todo ���Ͽ����� �о���� ���� �ƴ϶�, �޸� �ҽ��κ��� �д� �Լ��� 
/// �־�� �� �� �ϴ�.
////////////////////////////////////////////////////////////////////////////////

class cCsvFile
{
private:
    typedef std::vector<cCsvRow*> ROWS;

    ROWS m_Rows; ///< �� �÷���


public:
    /// \brief ������
    cCsvFile() {}

    /// \brief �Ҹ���
    virtual ~cCsvFile() { Destroy(); }


public:
    /// \brief ������ �̸��� CSV ������ �ε��Ѵ�.
    bool Load(const char* fileName, const char seperator=',', const char quote='"');

    /// \brief ������ �ִ� ������ CSV ���Ͽ��� �����Ѵ�.
    bool Save(const char* fileName, bool append=false, char seperator=',', char quote='"') const;

    /// \brief ��� �����͸� �޸𸮿��� �����Ѵ�.
    void Destroy();

    /// \brief �ش��ϴ� �ε����� ���� ��ȯ�Ѵ�.
    cCsvRow* operator [] (size_t index);

    /// \brief �ش��ϴ� �ε����� ���� ��ȯ�Ѵ�.
    const cCsvRow* operator [] (size_t index) const;

    /// \brief ���� ������ ��ȯ�Ѵ�.
    size_t GetRowCount() const { return m_Rows.size(); }


private:
    /// \brief ���� ������ ����
    cCsvFile(const cCsvFile&) {}

    /// \brief ���� ������ ����
    const cCsvFile& operator = (const cCsvFile&) { return *this; }
};


////////////////////////////////////////////////////////////////////////////////
/// \class cCsvTable
/// \brief CSV ������ �̿��� ���̺� �����͸� �ε��ϴ� ��찡 ������, �� Ŭ������ 
/// �� �۾��� �� �� ���� �ϱ� ���� ���� ��ƿ��Ƽ Ŭ������.
///
/// CSV ������ �ε��ϴ� ���, ���ڸ� �̿��� ���� �׼����ؾ� �ϴµ�, CSV 
/// ������ ������ �ٲ�� ���, �� ���ڵ��� ����������Ѵ�. �� �۾��� �� 
/// �Ű� ������ �䱸�ϴ� ���ٰ�, ������ �߻��ϱ� ����. �׷��Ƿ� ���ڷ� 
/// �׼����ϱ⺸�ٴ� ���ڿ��� �׼����ϴ� ���� �ణ �������� ���ٰ� �� �� �ִ�.
///
/// <b>sample</b>
/// <pre>
/// cCsvTable table;
///
/// table.alias(0, "ItemClass");
/// table.alias(1, "ItemType");
///
/// if (table.load("test.csv"))
/// {
///     while (table.next())
///     {
///         std::string item_class = table.AsString("ItemClass");
///         int         item_type  = table.AsInt("ItemType"); 
///     }
/// }
/// </pre>
////////////////////////////////////////////////////////////////////////////////

class cCsvTable
{
public :
    cCsvFile  m_File;   ///< CSV ���� ��ü
private:
    cCsvAlias m_Alias;  ///< ���ڿ��� �� �ε����� ��ȯ�ϱ� ���� ��ü
    int       m_CurRow; ///< ���� Ⱦ�� ���� �� ��ȣ


public:
    /// \brief ������
    cCsvTable();

    /// \brief �Ҹ���
    virtual ~cCsvTable();


public:
    /// \brief ������ �̸��� CSV ������ �ε��Ѵ�.
    bool Load(const char* fileName, const char seperator=',', const char quote='"');

    /// \brief ���� �׼����� ��, ���� ��� ����� �̸��� ����Ѵ�.
    void AddAlias(const char* name, size_t index) { m_Alias.AddAlias(name, index); }

    /// \brief ���� ������ �Ѿ��.
    bool Next();

    /// \brief ���� ���� �� ���ڸ� ��ȯ�Ѵ�.
    size_t ColCount() const;

    /// \brief �ε����� �̿��� int ������ ������ ��ȯ�Ѵ�.
    int AsInt(size_t index) const;

    /// \brief �ε����� �̿��� double ������ ������ ��ȯ�Ѵ�.
    double AsDouble(size_t index) const;

    /// \brief �ε����� �̿��� std::string ������ ������ ��ȯ�Ѵ�.
    const char* AsStringByIndex(size_t index) const;

    /// \brief �� �̸��� �̿��� int ������ ������ ��ȯ�Ѵ�.
    int AsInt(const char* name) const { return AsInt(m_Alias[name]); }

    /// \brief �� �̸��� �̿��� double ������ ������ ��ȯ�Ѵ�.
    double AsDouble(const char* name) const { return AsDouble(m_Alias[name]); }

    /// \brief �� �̸��� �̿��� std::string ������ ������ ��ȯ�Ѵ�.
    const char* AsString(const char* name) const { return AsStringByIndex(m_Alias[name]); }

    /// \brief alias�� ������ ��� �����͸� �����Ѵ�.
    void Destroy();


private:
    /// \brief ���� ���� ��ȯ�Ѵ�.
    const cCsvRow* const CurRow() const;

    /// \brief ���� ������ ����
    cCsvTable(const cCsvTable&) {}

    /// \brief ���� ������ ����
    const cCsvTable& operator = (const cCsvTable&) { return *this; }
};

#endif //__CSVFILE_H__
