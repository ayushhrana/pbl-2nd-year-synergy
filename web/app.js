var today = "";
var categories = [];
var people = [];
var groups = [];
var currentPage = "dashboard";
var currentGroupId = 0;
var currentFriendId = 0;
var simplifyDebts = true;

// ------------------------------------------------------------ basic helpers

async function api(action, body)
{
    const response = await fetch("/api/" + action,
    {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(body || {})
    });
    const result = await response.json();
    if (!result.ok)
    {
        throw new Error(result.error);
    }
    return result.data;
}

function esc(text)
{
    return String(text).replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;").replace(/"/g, "&quot;");
}

function rupees(paise)
{
    let sign = "";
    if (paise < 0)
    {
        sign = "-";
    }
    return sign + "Rs. " + (Math.abs(paise) / 100).toFixed(2);
}

function el(id)
{
    return document.getElementById(id);
}

function message(text, isError)
{
    if (!text)
    {
        el("message").innerHTML = "";
        return;
    }
    let kind = "info";
    if (isError)
    {
        kind = "error";
    }
    el("message").innerHTML = '<div class="' + kind + '">' + esc(text) + "</div>";
}

function openPanel(html)
{
    el("panel").innerHTML = html;
    el("panel").style.display = "block";
    window.scrollTo(0, 0);
}

function closePanel()
{
    el("panel").innerHTML = "";
    el("panel").style.display = "none";
}

async function run(task)
{
    message("");
    try
    {
        await task();
    }
    catch (error)
    {
        message(error.message, true);
    }
}

function row(cells, deleted)
{
    return { cells: cells, deleted: deleted === true };
}

function table(headers, rows)
{
    if (rows.length === 0)
    {
        return '<p class="small">Nothing to show.</p>';
    }
    let html = "<table><tr>";
    for (const header of headers)
    {
        html += "<th>" + header + "</th>";
    }
    html += "</tr>";
    for (const item of rows)
    {
        html += "<tr";
        if (item.deleted)
        {
            html += ' class="deleted"';
        }
        html += ">";
        for (const cell of item.cells)
        {
            html += "<td>" + cell + "</td>";
        }
        html += "</tr>";
    }
    return html + "</table>";
}

function categoryOptions(selected)
{
    let html = "";
    for (const name of categories)
    {
        let mark = "";
        if (name === selected)
        {
            mark = " selected";
        }
        html += '<option value="' + esc(name) + '"' + mark + ">" + esc(name) + "</option>";
    }
    return html;
}

function personName(id)
{
    for (const person of people)
    {
        if (person.id === id)
        {
            return person.name;
        }
    }
    return "Unknown";
}

function balanceLine(name, balance)
{
    if (balance > 0)
    {
        return esc(name) + ' owes you <b class="green">' + rupees(balance) + "</b>";
    }
    if (balance < 0)
    {
        return "You owe " + esc(name) + ' <b class="red">' + rupees(-balance) + "</b>";
    }
    return esc(name) + " is settled up";
}

async function loadBasics()
{
    categories = await api("categories/list");
    people = await api("people/list");
    groups = await api("groups/list");
}

// ------------------------------------------------------------ navigation

function setActive(name)
{
    const buttons = el("nav").getElementsByTagName("button");
    for (const button of buttons)
    {
        button.className = "";
    }
    let tab = name;
    if (name === "group")
    {
        tab = "groups";
    }
    if (name === "friend")
    {
        tab = "friends";
    }
    el("nav-" + tab).className = "active";
}

async function refresh()
{
    await loadBasics();
    await pages[currentPage]();
}

function go(name)
{
    closePanel();
    currentPage = name;
    setActive(name);
    run(refresh);
}

function openGroup(id)
{
    closePanel();
    currentGroupId = id;
    currentPage = "group";
    setActive("group");
    run(refresh);
}

function openFriend(id)
{
    closePanel();
    currentFriendId = id;
    currentPage = "friend";
    setActive("friend");
    run(refresh);
}

// ------------------------------------------------------------ dashboard

async function renderDashboard()
{
    const overview = await api("balances/overview");
    const report = await api("split/report", { groupId: -1 });

    let html = '<div class="boxes">';
    html += '<div class="box">You owe<span class="red">' + rupees(overview.youOwe) + "</span></div>";
    html += '<div class="box">You are owed<span class="green">' + rupees(overview.youAreOwed) + "</span></div>";
    html += '<div class="box">Total balance<span>' + rupees(overview.net) + "</span></div>";
    html += "</div>";

    html += '<div class="card"><button onclick="openSharedForm(0, null)">Add shared expense</button>';
    html += '<button onclick="openSettleForm(0, 0, 0, 0)">Settle up</button></div>';

    html += '<div class="card"><h2>Friends</h2>';
    const friendRows = [];
    for (const friend of overview.people)
    {
        let button = "";
        if (friend.balance !== 0)
        {
            button = '<button onclick="settleWithFriend(0, ' + friend.id + ', ' + friend.balance + ')">Settle up</button>';
        }
        friendRows.push(row(['<a class="link" onclick="openFriend(' + friend.id + ')">' + esc(friend.name) + "</a>", balanceLine(friend.name, friend.balance), button]));
    }
    html += table(["Friend", "Balance", ""], friendRows) + "</div>";

    html += '<div class="card"><h2>Groups</h2>';
    const groupRows = [];
    for (const group of overview.groups)
    {
        let text = "settled up";
        if (group.balance > 0)
        {
            text = 'you are owed <b class="green">' + rupees(group.balance) + "</b>";
        }
        if (group.balance < 0)
        {
            text = 'you owe <b class="red">' + rupees(-group.balance) + "</b>";
        }
        groupRows.push(row(['<a class="link" onclick="openGroup(' + group.id + ')">' + esc(group.name) + "</a>", text]));
    }
    html += table(["Group", "Your balance"], groupRows) + "</div>";

    html += '<div class="card"><h2>Your spending in shared expenses</h2>';
    html += "<p>Total paid by you: <b>" + rupees(report.paidByYou) + "</b> &nbsp; Your share: <b>" + rupees(report.yourShare) + "</b></p>";
    const categoryRows = [];
    for (const item of report.byCategory)
    {
        categoryRows.push(row([esc(item.category), rupees(item.total)]));
    }
    html += table(["Category", "Your share"], categoryRows) + "</div>";

    el("content").innerHTML = html;
}

// ------------------------------------------------------------ shared expense form

function groupMembers(groupId)
{
    if (groupId === 0)
    {
        return people;
    }
    for (const group of groups)
    {
        if (group.id === groupId)
        {
            return group.members;
        }
    }
    return [];
}

function shareValueText(type, value)
{
    if (type === "exact" || type === "percent")
    {
        return (value / 100).toFixed(2);
    }
    if (type === "shares")
    {
        return String(value);
    }
    return "";
}

function openSharedForm(groupId, expense)
{
    let data = { id: 0, groupId: groupId, description: "", amountText: "", category: categories[0], date: today,
                 paidBy: 1, splitType: "equal", note: "", shares: [] };
    let title = "Add a shared expense";
    if (expense !== null)
    {
        data = expense;
        data.amountText = (expense.amount / 100).toFixed(2);
        title = "Edit shared expense";
    }

    let groupOptions = '<option value="0">No group (friends only)</option>';
    for (const group of groups)
    {
        let mark = "";
        if (group.id === data.groupId)
        {
            mark = " selected";
        }
        groupOptions += '<option value="' + group.id + '"' + mark + ">" + esc(group.name) + "</option>";
    }

    let splitOptions = "";
    const types = ["equal", "exact", "percent", "shares"];
    const typeNames = ["Equally", "By exact amounts", "By percentages", "By shares"];
    for (let i = 0; i < types.length; i++)
    {
        let mark = "";
        if (types[i] === data.splitType)
        {
            mark = " selected";
        }
        splitOptions += '<option value="' + types[i] + '"' + mark + ">" + typeNames[i] + "</option>";
    }

    let html = "<h3>" + title + "</h3>";
    html += '<label>Description <input id="sDescription" value="' + esc(data.description) + '"></label>';
    html += '<label>Amount (Rs.) <input id="sAmount" size="10" value="' + esc(data.amountText) + '"></label>';
    html += '<label>Category <select id="sCategory">' + categoryOptions(data.category) + "</select></label>";
    html += '<label>Date <input type="date" id="sDate" value="' + esc(data.date) + '"></label><br>';
    html += '<label>Group <select id="sGroup" onchange="groupChanged()">' + groupOptions + "</select></label>";
    html += '<label>Paid by <select id="sPaid"></select></label>';
    html += '<label>Split <select id="sSplit" onchange="splitChanged()">' + splitOptions + "</select></label><br>";
    html += '<div id="sPeople"></div>';
    html += '<label>Note <input id="sNote" size="40" value="' + esc(data.note) + '"></label><br>';
    html += '<button onclick="run(function () { return saveShared(' + data.id + '); })">Save</button>';
    html += '<button onclick="closePanel()">Cancel</button>';
    openPanel(html);
    buildPeople(data);
}

function buildPeople(data)
{
    const members = groupMembers(Number(el("sGroup").value));
    let paidOptions = "";
    let rows = "";
    for (const person of members)
    {
        let paidMark = "";
        if (person.id === data.paidBy)
        {
            paidMark = " selected";
        }
        paidOptions += '<option value="' + person.id + '"' + paidMark + ">" + esc(person.name) + "</option>";

        let checked = true;
        let value = "";
        if (data.shares.length > 0)
        {
            checked = false;
            for (const share of data.shares)
            {
                if (share.personId === person.id)
                {
                    checked = true;
                    value = shareValueText(data.splitType, share.inputValue);
                }
            }
        }
        let checkMark = "";
        if (checked)
        {
            checkMark = " checked";
        }
        rows += '<div class="row"><label><input type="checkbox" id="chk' + person.id + '"' + checkMark + "> " + esc(person.name) +
                '</label> <input class="valueBox" id="val' + person.id + '" value="' + esc(value) + '"></div>';
    }
    el("sPaid").innerHTML = paidOptions;
    el("sPeople").innerHTML = rows;
    splitChanged();
}

function groupChanged()
{
    buildPeople({ paidBy: 1, splitType: el("sSplit").value, shares: [] });
}

function splitChanged()
{
    const type = el("sSplit").value;
    const members = groupMembers(Number(el("sGroup").value));
    for (const person of members)
    {
        const box = el("val" + person.id);
        if (type === "equal")
        {
            box.style.display = "none";
        }
        else
        {
            box.style.display = "inline";
            if (type === "exact")
            {
                box.placeholder = "amount";
            }
            if (type === "percent")
            {
                box.placeholder = "percent";
            }
            if (type === "shares")
            {
                box.placeholder = "shares";
                if (box.value === "")
                {
                    box.value = "1";
                }
            }
        }
    }
}

async function saveShared(id)
{
    const members = groupMembers(Number(el("sGroup").value));
    const inputs = [];
    for (const person of members)
    {
        if (el("chk" + person.id).checked)
        {
            inputs.push({ personId: person.id, value: el("val" + person.id).value });
        }
    }

    const body =
    {
        groupId: Number(el("sGroup").value),
        description: el("sDescription").value,
        category: el("sCategory").value,
        amount: el("sAmount").value,
        date: el("sDate").value,
        paidBy: Number(el("sPaid").value),
        splitType: el("sSplit").value,
        note: el("sNote").value,
        inputs: inputs
    };

    if (id > 0)
    {
        body.id = id;
        await api("shared/update", body);
    }
    else
    {
        await api("shared/add", body);
    }
    closePanel();
    await refresh();
    message("Expense saved.", false);
}

async function editShared(id)
{
    const list = await api("shared/list", { groupId: -1, includeDeleted: true });
    for (const expense of list)
    {
        if (expense.id === id)
        {
            openSharedForm(expense.groupId, expense);
            return;
        }
    }
}

async function deleteShared(id)
{
    await api("shared/delete", { id: id });
    await refresh();
    message("Expense deleted. You can restore it from the list.", false);
}

async function restoreShared(id)
{
    await api("shared/restore", { id: id });
    await refresh();
    message("Expense restored.", false);
}

// ------------------------------------------------------------ settle up

function settleWithFriend(groupId, friendId, balance)
{
    if (balance > 0)
    {
        openSettleForm(groupId, friendId, 1, balance);
    }
    else
    {
        openSettleForm(groupId, 1, friendId, -balance);
    }
}

function openSettleForm(groupId, fromId, toId, paise)
{
    const members = groupMembers(groupId);
    let fromOptions = "";
    let toOptions = "";
    for (const person of members)
    {
        let fromMark = "";
        let toMark = "";
        if (person.id === fromId)
        {
            fromMark = " selected";
        }
        if (person.id === toId)
        {
            toMark = " selected";
        }
        fromOptions += '<option value="' + person.id + '"' + fromMark + ">" + esc(person.name) + "</option>";
        toOptions += '<option value="' + person.id + '"' + toMark + ">" + esc(person.name) + "</option>";
    }

    let amountText = "";
    if (paise > 0)
    {
        amountText = (paise / 100).toFixed(2);
    }

    let html = "<h3>Settle up (" + esc(groupId === 0 ? "no group" : "group") + ")</h3>";
    html += '<label>Who paid <select id="tFrom">' + fromOptions + "</select></label>";
    html += '<label>Paid to <select id="tTo">' + toOptions + "</select></label>";
    html += '<label>Amount (Rs.) <input id="tAmount" size="10" value="' + amountText + '"></label>';
    html += '<label>Date <input type="date" id="tDate" value="' + today + '"></label>';
    html += '<label>Note <input id="tNote"></label><br>';
    html += '<button onclick="run(function () { return saveSettle(' + groupId + '); })">Save payment</button>';
    html += '<button onclick="closePanel()">Cancel</button>';
    openPanel(html);
}

async function saveSettle(groupId)
{
    await api("settle/add",
    {
        groupId: groupId,
        fromId: Number(el("tFrom").value),
        toId: Number(el("tTo").value),
        amount: el("tAmount").value,
        date: el("tDate").value,
        note: el("tNote").value
    });
    closePanel();
    await refresh();
    message("Payment recorded.", false);
}

async function deleteSettlement(id)
{
    await api("settle/delete", { id: id });
    await refresh();
    message("Payment deleted.", false);
}

// ------------------------------------------------------------ groups

async function renderGroups()
{
    let html = '<div class="card"><h2>Create a group</h2>';
    html += '<label>Group name <input id="gName"></label><br>';
    html += "<div>Add friends: ";
    for (const person of people)
    {
        if (person.id !== 1)
        {
            html += '<label><input type="checkbox" id="gChk' + person.id + '"> ' + esc(person.name) + "</label>";
        }
    }
    html += "</div>";
    html += '<label>New people (names separated by commas) <input id="gNew" size="30"></label><br>';
    html += '<button onclick="run(createGroup)">Create group</button></div>';

    html += '<div class="card"><h2>Your groups</h2>';
    const rows = [];
    for (const group of groups)
    {
        let text = "settled up";
        if (group.yourBalance > 0)
        {
            text = 'you are owed <b class="green">' + rupees(group.yourBalance) + "</b>";
        }
        if (group.yourBalance < 0)
        {
            text = 'you owe <b class="red">' + rupees(-group.yourBalance) + "</b>";
        }
        rows.push(row(['<a class="link" onclick="openGroup(' + group.id + ')">' + esc(group.name) + "</a>",
                       group.members.length, rupees(group.totalSpent), text]));
    }
    html += table(["Group", "Members", "Total spent", "Your balance"], rows) + "</div>";
    el("content").innerHTML = html;
}

async function createGroup()
{
    const ids = [];
    for (const person of people)
    {
        if (person.id !== 1 && el("gChk" + person.id).checked)
        {
            ids.push(person.id);
        }
    }

    const names = el("gNew").value.split(",");
    for (const name of names)
    {
        if (name.trim() !== "")
        {
            const added = await api("people/add", { name: name.trim() });
            ids.push(added.id);
        }
    }

    const result = await api("groups/create", { name: el("gName").value, members: ids });
    openGroup(result.id);
}

async function renderGroup()
{
    const data = await api("groups/get", { groupId: currentGroupId, simplify: simplifyDebts });
    const group = data.group;

    let html = '<div class="card"><h2>' + esc(group.name) + "</h2>";
    html += "<p>Total spent: <b>" + rupees(data.totalSpent) + "</b> &nbsp; Paid by you: <b>" + rupees(data.paidByYou) +
            "</b> &nbsp; Your share: <b>" + rupees(data.yourShare) + "</b></p>";
    html += '<button onclick="openSharedForm(' + group.id + ', null)">Add expense</button>';
    html += '<button onclick="openSettleForm(' + group.id + ', 0, 0, 0)">Settle up</button>';
    html += '<button onclick="run(function () { return deleteGroup(' + group.id + '); })">Delete group</button></div>';

    html += '<div class="card"><h3>Members</h3><p>';
    for (const member of group.members)
    {
        html += esc(member.name);
        if (member.id !== 1)
        {
            html += ' <button onclick="run(function () { return removeMember(' + group.id + ', ' + member.id + '); })">remove</button>';
        }
        html += " &nbsp; ";
    }
    html += "</p>";

    let options = "";
    for (const person of people)
    {
        let isMember = false;
        for (const member of group.members)
        {
            if (member.id === person.id)
            {
                isMember = true;
            }
        }
        if (!isMember)
        {
            options += '<option value="' + person.id + '">' + esc(person.name) + "</option>";
        }
    }
    if (options !== "")
    {
        html += '<select id="addMemberSelect">' + options + "</select>";
        html += '<button onclick="run(function () { return addMember(' + group.id + '); })">Add friend to group</button> ';
    }
    html += '<input id="newMemberName" placeholder="or a new name">';
    html += '<button onclick="run(function () { return addNewMember(' + group.id + '); })">Add new person</button></div>';

    html += '<div class="card"><h3>Balances</h3>';
    const balanceRows = [];
    for (const item of data.balances)
    {
        let text = "settled up";
        if (item.balance > 0)
        {
            text = 'gets back <b class="green">' + rupees(item.balance) + "</b>";
        }
        if (item.balance < 0)
        {
            text = 'owes <b class="red">' + rupees(-item.balance) + "</b>";
        }
        balanceRows.push(row([esc(item.name), text]));
    }
    html += table(["Person", "Balance"], balanceRows);

    let checked = "";
    if (data.simplified)
    {
        checked = " checked";
    }
    html += '<p><label><input type="checkbox" id="simplify"' + checked + ' onchange="toggleSimplify()"> Simplify debts (fewest payments)</label></p>';
    const debtRows = [];
    for (const debt of data.debts)
    {
        debtRows.push(row([esc(debt.fromName) + " pays " + esc(debt.toName), rupees(debt.amount),
                           '<button onclick="openSettleForm(' + group.id + ', ' + debt.fromId + ', ' + debt.toId + ', ' + debt.amount + ')">Settle</button>']));
    }
    html += table(["Who pays whom", "Amount", ""], debtRows) + "</div>";

    html += '<div class="card"><h3>Expenses</h3>';
    const expenseRows = [];
    for (const expense of data.expenses)
    {
        let yourShare = "-";
        for (const share of expense.shares)
        {
            if (share.personId === 1)
            {
                yourShare = rupees(share.owed);
            }
        }
        let buttons = '<button onclick="run(function () { return editShared(' + expense.id + '); })">Edit</button>';
        if (expense.deleted)
        {
            buttons = '<button onclick="run(function () { return restoreShared(' + expense.id + '); })">Restore</button>';
        }
        else
        {
            buttons += '<button onclick="run(function () { return deleteShared(' + expense.id + '); })">Delete</button>';
        }
        expenseRows.push(row([esc(expense.date), esc(expense.description), esc(expense.category), esc(expense.paidByName),
                              rupees(expense.amount), esc(expense.splitType), yourShare, buttons], expense.deleted));
    }
    html += table(["Date", "Description", "Category", "Paid by", "Amount", "Split", "Your share", ""], expenseRows) + "</div>";

    html += '<div class="card"><h3>Payments</h3>';
    const paymentRows = [];
    for (const payment of data.settlements)
    {
        let button = "";
        if (!payment.deleted)
        {
            button = '<button onclick="run(function () { return deleteSettlement(' + payment.id + '); })">Delete</button>';
        }
        paymentRows.push(row([esc(payment.date), esc(payment.fromName) + " paid " + esc(payment.toName), rupees(payment.amount), esc(payment.note), button], payment.deleted));
    }
    html += table(["Date", "Payment", "Amount", "Note", ""], paymentRows) + "</div>";

    el("content").innerHTML = html;
}

function toggleSimplify()
{
    simplifyDebts = el("simplify").checked;
    run(refresh);
}

async function addMember(groupId)
{
    await api("groups/addMember", { groupId: groupId, personId: Number(el("addMemberSelect").value) });
    await refresh();
}

async function addNewMember(groupId)
{
    const added = await api("people/add", { name: el("newMemberName").value });
    await api("groups/addMember", { groupId: groupId, personId: added.id });
    await refresh();
}

async function removeMember(groupId, personId)
{
    await api("groups/removeMember", { groupId: groupId, personId: personId });
    await refresh();
}

async function deleteGroup(groupId)
{
    if (!confirm("Delete this group and all its expenses? (Only possible when everything is settled up)"))
    {
        return;
    }
    await api("groups/delete", { groupId: groupId });
    go("groups");
}

// ------------------------------------------------------------ friends

async function renderFriends()
{
    const overview = await api("balances/overview");

    let html = '<div class="card"><h2>Add a friend</h2>';
    html += '<input id="fName" placeholder="Name"> <button onclick="run(addFriend)">Add friend</button></div>';

    html += '<div class="card"><h2>Your friends</h2>';
    const rows = [];
    for (const friend of overview.people)
    {
        rows.push(row(['<a class="link" onclick="openFriend(' + friend.id + ')">' + esc(friend.name) + "</a>", balanceLine(friend.name, friend.balance)]));
    }
    html += table(["Friend", "Balance"], rows) + "</div>";
    el("content").innerHTML = html;
}

async function addFriend()
{
    await api("people/add", { name: el("fName").value });
    await refresh();
    message("Friend added.", false);
}

async function renderFriend()
{
    const data = await api("balances/person", { personId: currentFriendId });
    const name = data.person.name;

    let html = '<div class="card"><h2>' + esc(name) + "</h2><p>" + balanceLine(name, data.balance) + "</p>";
    html += '<button onclick="openSharedForm(0, null)">Add expense</button>';
    html += '<button onclick="run(function () { return renameFriend(' + data.person.id + '); })">Rename</button>';
    html += '<button onclick="run(function () { return removeFriend(' + data.person.id + '); })">Remove friend</button></div>';

    html += '<div class="card"><h3>Balance in each group</h3>';
    const groupRows = [];
    for (const item of data.groups)
    {
        groupRows.push(row([esc(item.groupName), balanceLine(name, item.balance),
                            '<button onclick="settleWithFriend(' + item.groupId + ', ' + data.person.id + ', ' + item.balance + ')">Settle up</button>']));
    }
    html += table(["Group", "Balance", ""], groupRows) + "</div>";

    html += '<div class="card"><h3>History</h3>';
    const historyRows = [];
    for (const item of data.history)
    {
        let effect = rupees(item.effect);
        if (item.effect > 0)
        {
            effect = '<span class="green">+' + rupees(item.effect) + "</span>";
        }
        if (item.effect < 0)
        {
            effect = '<span class="red">' + rupees(item.effect) + "</span>";
        }
        historyRows.push(row([esc(item.date), esc(item.text), esc(item.groupName), rupees(item.amount), effect]));
    }
    html += table(["Date", "What", "Group", "Total", "Effect on balance"], historyRows);
    html += '<p class="small">Green: ' + esc(name) + " owes you more. Red: you owe " + esc(name) + " more.</p></div>";
    el("content").innerHTML = html;
}

async function renameFriend(id)
{
    const newName = prompt("New name:");
    if (newName === null)
    {
        return;
    }
    await api("people/rename", { id: id, name: newName });
    await refresh();
}

async function removeFriend(id)
{
    if (!confirm("Remove this friend?"))
    {
        return;
    }
    await api("people/remove", { id: id });
    go("friends");
}

// ------------------------------------------------------------ my expenses

function personalFields(prefix, e)
{
    let data = { amountText: "", category: categories[0], date: today, note: "", place: "", mode: "cash" };
    if (e !== null)
    {
        data = e;
        data.amountText = (e.amount / 100).toFixed(2);
    }

    let cashMark = "";
    let onlineMark = "";
    if (data.mode === "online")
    {
        onlineMark = " selected";
    }
    else
    {
        cashMark = " selected";
    }

    let html = '<label>Amount (Rs.) <input id="' + prefix + 'Amount" size="10" value="' + esc(data.amountText) + '"></label>';
    html += '<label>Category <select id="' + prefix + 'Category">' + categoryOptions(data.category) + "</select></label>";
    html += '<label>Date <input type="date" id="' + prefix + 'Date" value="' + esc(data.date) + '"></label>';
    html += '<label>Payment <select id="' + prefix + 'Mode"><option value="cash"' + cashMark + '>Cash</option><option value="online"' + onlineMark + ">Online</option></select></label><br>";
    html += '<label>Note <input id="' + prefix + 'Note" value="' + esc(data.note) + '"></label>';
    html += '<label>Place <input id="' + prefix + 'Place" value="' + esc(data.place) + '"></label>';
    return html;
}

function readPersonal(prefix)
{
    return {
        amount: el(prefix + "Amount").value,
        category: el(prefix + "Category").value,
        date: el(prefix + "Date").value,
        note: el(prefix + "Note").value,
        place: el(prefix + "Place").value,
        mode: el(prefix + "Mode").value
    };
}

async function renderExpenses()
{
    let html = '<div class="card"><h2>Add an expense</h2>' + personalFields("n", null);
    html += '<br><button onclick="run(function () { return addPersonal(false); })">Add expense</button></div>';

    html += '<div class="card"><h2>My expenses</h2>';
    html += '<button id="undoButton" onclick="run(undoLast)">Undo</button><button id="redoButton" onclick="run(redoLast)">Redo</button><br>';
    html += '<label>Sort by <select id="sortBy"><option value="id">ID</option><option value="date">Date</option><option value="amount">Amount</option></select></label>';
    html += '<label>Order <select id="sortOrder"><option value="asc">Smallest first</option><option value="desc">Biggest first</option></select></label>';
    html += '<button onclick="run(loadExpenseList)">Show</button><br>';
    html += '<label>Search note or place <input id="searchText"></label><button onclick="run(doSearch)">Search</button><br>';
    html += "<b>Filter:</b> ";
    html += '<label>Category <select id="fCategory"><option value="">Any</option>' + categoryOptions("") + "</select></label>";
    html += '<label>From <input type="date" id="fFrom"></label><label>To <input type="date" id="fTo"></label>';
    html += '<label>Payment <select id="fMode"><option value="">Any</option><option value="cash">Cash</option><option value="online">Online</option></select></label>';
    html += '<label>Min Rs. <input id="fMin" size="6"></label><label>Max Rs. <input id="fMax" size="6"></label>';
    html += '<label>Place has <input id="fPlace" size="10"></label><button onclick="run(doFilter)">Filter</button>';
    html += '<div id="expList"></div></div>';

    html += '<div class="card"><h2>Statistics</h2><div id="expStats"></div></div>';

    html += '<div class="card"><h2>Categories</h2><p id="catList"></p>';
    html += '<input id="newCategory" placeholder="New category"> <button onclick="run(addCategory)">Add category</button></div>';
    el("content").innerHTML = html;

    await loadExpenseList();
    await loadStats();
    await updateUndoButtons();
    showCategoryList();
}

function showCategoryList()
{
    let text = "";
    for (const name of categories)
    {
        text += esc(name) + " &nbsp; ";
    }
    el("catList").innerHTML = text;
}

async function addCategory()
{
    await api("categories/add", { name: el("newCategory").value });
    await refresh();
    message("Category added.", false);
}

function showExpenseList(items)
{
    const rows = [];
    for (const item of items)
    {
        rows.push(row([item.id, esc(item.date), esc(item.amountText), esc(item.category), esc(item.note), esc(item.place), esc(item.mode),
                       '<button onclick="run(function () { return editPersonal(' + item.id + '); })">Edit</button>' +
                       '<button onclick="run(function () { return deletePersonal(' + item.id + '); })">Delete</button>']));
    }
    el("expList").innerHTML = table(["ID", "Date", "Amount", "Category", "Note", "Place", "Payment", ""], rows);
}

async function loadExpenseList()
{
    showExpenseList(await api("expenses/list", { sortBy: el("sortBy").value, order: el("sortOrder").value }));
}

async function doSearch()
{
    showExpenseList(await api("expenses/search", { text: el("searchText").value }));
}

async function doFilter()
{
    showExpenseList(await api("expenses/filter",
    {
        category: el("fCategory").value,
        from: el("fFrom").value,
        to: el("fTo").value,
        mode: el("fMode").value,
        min: el("fMin").value,
        max: el("fMax").value,
        place: el("fPlace").value
    }));
}

async function loadStats()
{
    const stats = await api("expenses/stats");
    let html = "<p>Number of expenses: <b>" + stats.count + "</b> &nbsp; Total: <b>" + esc(stats.totalText) + "</b> &nbsp; Average: <b>" + esc(stats.averageText) + "</b></p>";
    if (stats.smallest !== null)
    {
        html += "<p>Smallest: " + esc(stats.smallest.note) + " (" + esc(stats.smallest.amountText) + ") &nbsp; Largest: " +
                esc(stats.largest.note) + " (" + esc(stats.largest.amountText) + ")</p>";
    }
    if (stats.duplicates.length > 0)
    {
        html += "<p><b>Possible duplicates:</b></p>";
        const rows = [];
        for (const item of stats.duplicates)
        {
            rows.push(row([item.id, esc(item.date), esc(item.amountText), esc(item.category), esc(item.note)]));
        }
        html += table(["ID", "Date", "Amount", "Category", "Note"], rows);
    }
    el("expStats").innerHTML = html;
}

async function updateUndoButtons()
{
    const status = await api("undo/status");
    el("undoButton").disabled = !status.canUndo;
    el("redoButton").disabled = !status.canRedo;
}

async function reloadExpenses()
{
    await loadBasics();
    await loadExpenseList();
    await loadStats();
    await updateUndoButtons();
}

async function addPersonal(force)
{
    const body = readPersonal("n");
    if (force)
    {
        body.force = true;
    }
    const result = await api("expenses/add", body);
    if (result.duplicate)
    {
        if (confirm("This looks like a duplicate of an expense you already added. Add it anyway?"))
        {
            await addPersonal(true);
        }
        return;
    }
    el("nAmount").value = "";
    el("nNote").value = "";
    el("nPlace").value = "";
    await reloadExpenses();
    message("Expense added with ID " + result.id + ".", false);
}

async function editPersonal(id)
{
    const expense = await api("expenses/get", { id: id });
    let html = "<h3>Edit expense " + id + "</h3>" + personalFields("e", expense);
    html += '<br><button onclick="run(function () { return savePersonal(' + id + '); })">Save</button><button onclick="closePanel()">Cancel</button>';
    openPanel(html);
}

async function savePersonal(id)
{
    const body = readPersonal("e");
    body.id = id;
    await api("expenses/update", body);
    closePanel();
    await reloadExpenses();
    message("Expense updated.", false);
}

async function deletePersonal(id)
{
    if (!confirm("Delete expense " + id + "? You can undo it."))
    {
        return;
    }
    await api("expenses/delete", { id: id });
    await reloadExpenses();
    message("Expense deleted. Use Undo to bring it back.", false);
}

async function undoLast()
{
    await api("undo");
    await reloadExpenses();
}

async function redoLast()
{
    await api("redo");
    await reloadExpenses();
}

// ------------------------------------------------------------ analytics

async function renderAnalytics()
{
    const firstDay = today.substring(0, 8) + "01";

    let html = '<div class="card"><h2>Analytics of my expenses</h2>';
    html += '<label>From <input type="date" id="aFrom" value="' + firstDay + '"></label>';
    html += '<label>To <input type="date" id="aTo" value="' + today + '"></label><br>';
    html += '<button onclick="run(showTotal)">Total</button>';
    html += '<button onclick="run(showByCategory)">By category</button>';
    html += '<button onclick="run(showByMode)">By payment mode</button>';
    html += '<button onclick="run(showReport)">Full report</button><br>';
    html += '<label>Summary <select id="aPeriod"><option value="daily">Daily</option><option value="weekly">Weekly (Monday)</option><option value="monthly" selected>Monthly</option></select></label>';
    html += '<button onclick="run(showSummary)">Show summary</button><br>';
    html += '<label>Top <input id="aTopN" size="3" value="3"> categories</label><button onclick="run(showTop)">Show top</button><br>';
    html += '<label>Budget Rs. <input id="aLimit" size="8"></label>';
    html += '<label>Category <select id="aBudgetCategory"><option value="">All</option>' + categoryOptions("") + "</select></label>";
    html += '<button onclick="run(showBudget)">Check budget</button><br>';
    html += "<b>Compare with another period:</b> ";
    html += '<label>From <input type="date" id="bFrom"></label><label>To <input type="date" id="bTo"></label>';
    html += '<button onclick="run(showCompare)">Compare</button><br>';
    html += "<b>Quick total inside the range above (prefix sums):</b> ";
    html += '<label>From <input type="date" id="qFrom"></label><label>To <input type="date" id="qTo"></label>';
    html += '<button onclick="run(showPrefix)">Get total</button></div>';
    html += '<div class="card" id="aResult"><p class="small">Choose a range and press a button.</p></div>';
    el("content").innerHTML = html;
}

function range()
{
    return { from: el("aFrom").value, to: el("aTo").value };
}

function showResult(html)
{
    el("aResult").innerHTML = html;
}

function totalsTable(items, keyName, title)
{
    const rows = [];
    for (const item of items)
    {
        rows.push(row([esc(item[keyName]), esc(item.totalText)]));
    }
    return table([title, "Total"], rows);
}

async function showTotal()
{
    const result = await api("analytics/total", range());
    showResult("<h3>Total spending</h3><p><b>" + esc(result.totalText) + "</b></p>");
}

async function showByCategory()
{
    showResult("<h3>Spending by category</h3>" + totalsTable(await api("analytics/categories", range()), "category", "Category"));
}

async function showByMode()
{
    showResult("<h3>Spending by payment mode</h3>" + totalsTable(await api("analytics/modes", range()), "mode", "Mode"));
}

async function showSummary()
{
    const body = range();
    body.period = el("aPeriod").value;
    showResult("<h3>" + esc(body.period) + " summary</h3>" + totalsTable(await api("analytics/summary", body), "period", "Period"));
}

async function showTop()
{
    const body = range();
    body.n = Number(el("aTopN").value);
    showResult("<h3>Top categories</h3>" + totalsTable(await api("analytics/top", body), "category", "Category"));
}

async function showBudget()
{
    const body = range();
    body.limit = el("aLimit").value;
    body.category = el("aBudgetCategory").value;
    const result = await api("analytics/budget", body);
    let status = '<b class="green">You are within the budget.</b>';
    if (result.exceeded)
    {
        status = '<b class="red">The budget is exceeded.</b>';
    }
    showResult("<h3>Budget</h3><p>Limit: " + esc(result.limitText) + "<br>Spent: " + esc(result.spentText) +
               "<br>Remaining: " + esc(result.remainingText) + "<br>" + status + "</p>");
}

async function showCompare()
{
    const body = range();
    const result = await api("analytics/compare",
    {
        aFrom: body.from,
        aTo: body.to,
        bFrom: el("bFrom").value,
        bTo: el("bTo").value
    });
    let percent = "";
    if (result.hasPercent)
    {
        percent = " (" + result.percent + "%)";
    }
    showResult("<h3>Comparison</h3><p>First period: " + esc(result.firstText) + "<br>Second period: " + esc(result.secondText) +
               "<br>Spending " + esc(result.trend) + " by " + esc(result.differenceText) + percent + "</p>");
}

async function showReport()
{
    const report = await api("analytics/report", range());
    let html = "<h3>Report " + esc(report.from) + " to " + esc(report.to) + "</h3>";
    html += "<p>Expenses: <b>" + report.count + "</b> &nbsp; Days: <b>" + report.days + "</b> &nbsp; Total: <b>" + esc(report.totalText) +
            "</b> &nbsp; Average per day: <b>" + esc(report.averagePerDayText) + "</b></p>";
    if (report.topCategory !== "")
    {
        html += "<p>Top category: <b>" + esc(report.topCategory) + "</b></p>";
    }
    html += totalsTable(report.categories, "category", "Category") + totalsTable(report.modes, "mode", "Payment mode");
    showResult(html);
}

async function showPrefix()
{
    const body = range();
    body.queryFrom = el("qFrom").value;
    body.queryTo = el("qTo").value;
    const result = await api("analytics/prefix", body);
    showResult("<h3>Total from the prefix sums</h3><p><b>" + esc(result.totalText) + "</b> (array built for " + result.days + " days)</p>");
}

// ------------------------------------------------------------ calendar

async function renderCalendar()
{
    const year = Number(today.substring(0, 4));
    const month = Number(today.substring(5, 7));

    let html = '<div class="card"><h2>Calendar</h2>';
    html += '<label>Day <input type="date" id="cDay" value="' + today + '"></label><button onclick="run(showDay)">Show day</button><br>';
    html += '<label>Year <input id="cYear" size="5" value="' + year + '"></label><label>Month (1-12) <input id="cMonth" size="3" value="' + month + '"></label>';
    html += '<button onclick="run(showMonth)">Show month</button><br>';
    html += '<label>From <input type="date" id="cFrom"></label><label>To <input type="date" id="cTo"></label><button onclick="run(showRange)">Show range</button></div>';
    html += '<div class="card" id="cResult"><p class="small">Choose a day, month or range.</p></div>';
    el("content").innerHTML = html;
}

function showCalendarList(title, items)
{
    const rows = [];
    let total = 0;
    for (const item of items)
    {
        total += item.amount;
        rows.push(row([esc(item.date), esc(item.amountText), esc(item.category), esc(item.note), esc(item.place), esc(item.mode)]));
    }
    el("cResult").innerHTML = "<h3>" + esc(title) + "</h3>" + table(["Date", "Amount", "Category", "Note", "Place", "Payment"], rows) +
                              "<p>Total: <b>" + rupees(total) + "</b></p>";
}

async function showDay()
{
    const day = el("cDay").value;
    showCalendarList("Expenses on " + day, await api("calendar/day", { date: day }));
}

async function showMonth()
{
    const year = Number(el("cYear").value);
    const month = Number(el("cMonth").value);
    showCalendarList("Expenses in " + year + "-" + month, await api("calendar/month", { year: year, month: month }));
}

async function showRange()
{
    const fromDate = el("cFrom").value;
    const toDate = el("cTo").value;
    showCalendarList("Expenses from " + fromDate + " to " + toDate, await api("calendar/range", { from: fromDate, to: toDate }));
}

// ------------------------------------------------------------ recurring expenses

async function renderRecurring()
{
    let html = '<div class="card"><h2>Add a recurring expense</h2>' + personalFields("r", null).replace('id="rDate"', 'id="rDate" style="display:none"');
    html += '<label>Repeats <select id="rFrequency"><option value="daily">Daily</option><option value="weekly">Weekly</option><option value="monthly" selected>Monthly</option></select></label>';
    html += '<label>Start <input type="date" id="rStart" value="' + today + '"></label>';
    html += '<label>End (optional) <input type="date" id="rEnd"></label><br>';
    html += '<button onclick="run(addRule)">Save recurring expense</button></div>';

    html += '<div class="card"><h2>My recurring expenses</h2><div id="ruleList"></div><p id="nextDue" class="small"></p></div>';

    html += '<div class="card"><h2>Confirm due expenses</h2>';
    html += '<p class="small">A recurring expense is only planned. It becomes a real expense after you press Record.</p>';
    html += '<label>Check what is due up to <input type="date" id="dueDate" value="' + today + '"></label>';
    html += '<button onclick="run(checkDue)">Check</button><div id="dueList"></div></div>';
    el("content").innerHTML = html;
    await loadRules();
    await showPending(await api("recurring/due", { today: today }));
}

async function loadRules()
{
    const rules = await api("recurring/list");
    const rows = [];
    for (const rule of rules)
    {
        let end = "no end";
        if (rule.hasEnd)
        {
            end = rule.end;
        }
        rows.push(row([rule.id, esc(rule.amountText), esc(rule.category), esc(rule.note), esc(rule.frequency), esc(rule.start), esc(end)]));
    }
    el("ruleList").innerHTML = table(["ID", "Amount", "Category", "Note", "Repeats", "Start", "End"], rows);

    const next = await api("recurring/next");
    if (next.has)
    {
        el("nextDue").innerHTML = "Next due: " + esc(next.occurrence.due) + " (" + esc(next.occurrence.note) + ", " + esc(next.occurrence.amountText) + ")";
    }
    else
    {
        el("nextDue").innerHTML = "Nothing is scheduled.";
    }
}

async function addRule()
{
    const body = readPersonal("r");
    body.frequency = el("rFrequency").value;
    body.start = el("rStart").value;
    body.end = el("rEnd").value;
    await api("recurring/add", body);
    await renderRecurring();
    message("Recurring expense saved.", false);
}

async function showPending(list)
{
    const rows = [];
    for (const item of list)
    {
        rows.push(row([esc(item.due), esc(item.amountText), esc(item.category), esc(item.note),
                       '<button onclick="run(function () { return decide(' + item.ruleId + ', ' + item.index + ', true); })">Record</button>' +
                       '<button onclick="run(function () { return decide(' + item.ruleId + ', ' + item.index + ', false); })">Skip</button>']));
    }
    el("dueList").innerHTML = table(["Due date", "Amount", "Category", "Note", ""], rows);
}

async function checkDue()
{
    await showPending(await api("recurring/due", { today: el("dueDate").value }));
    await loadRules();
}

async function decide(ruleId, index, accept)
{
    await api("recurring/decide", { ruleId: ruleId, index: index, accept: accept });
    await showPending(await api("recurring/due", { today: el("dueDate").value }));
    await loadRules();
    if (accept)
    {
        message("Expense recorded. You can see it in My Expenses.", false);
    }
}

// ------------------------------------------------------------ activity

async function renderActivity()
{
    const list = await api("activity", { limit: 100 });
    const rows = [];
    for (const item of list)
    {
        rows.push(row([esc(item.date), esc(item.text)]));
    }
    el("content").innerHTML = '<div class="card"><h2>Recent activity</h2>' + table(["Date", "What happened"], rows) + "</div>";
}

// ------------------------------------------------------------ start

var pages =
{
    dashboard: renderDashboard,
    expenses: renderExpenses,
    groups: renderGroups,
    friends: renderFriends,
    analytics: renderAnalytics,
    calendar: renderCalendar,
    recurring: renderRecurring,
    activity: renderActivity,
    group: renderGroup,
    friend: renderFriend
};

async function start()
{
    const info = await api("today");
    today = info.today;
    go("dashboard");
}

run(start);
