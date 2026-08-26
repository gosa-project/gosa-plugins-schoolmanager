{include file="`$phases_path`../common/fontawesome.tpl" inline}

<div class="card-content-scroll">
{if !isset($empty_schoolmgr_groups_now_phase1_done)} <!-- Phase1 flow control -->
<div class="row">
    <div class="col s12">
    <h3>{t}Step 1:{/t} {t}Empty SchoolManager administered standard (POSIX) groups{/t}</h3>

    <p>
        {t}The following list shows all POSIX groups (classes, courses and teachers' subjects) managed by SchoolManager. You are about to empty all listed groups, i.e.: drop all members from those groups.{/t}
    </p>

    <p>
        {t}With the next import of teachers or students, many of these groups will get automatically re-populated with members. If you are at the beginning of a new school year, you want to do this!{/t}
    </p>

    <p>
        {t}Flush all members from the listed course, class and subject (POSIX) groups now? After you have clicked this button, the process cannot be reverted! Emptying all groups may take some time, so please be patient while waiting…{/t}
    </p>

    <button class="btn-small primary" name="empty_schoolmgr_groups_now_phase2" type="submit">{t}Really empty all POSIX groups listed below, now!{/t}</button>

    <hr class="divider">

    {foreach from=$data_groups item=group key=key}
    <table summary="{t 1=$group['cn'][0]}POSIX Group: %1{/t}">
        <thead>
            <tr>
            <th style="width: 20%;">
                {t}Group Name{/t}
            </th>
            <th style="width: 30%;">
                {t}Description{/t}
            </th>
            <th>
                {t}Members (to be removed){/t}
            </th>
            </tr>
        </thead>
        <tbody>
            <tr>
                <td>{$group['cn'][0]}</td>
                <td>{$group['description'][0]}</td>
                <td>
                {if isset($group['memberUid']) && (count($group['memberUid']) > 0)}
                    {foreach from=$group['memberUid'] item=value key=key}
                        {$value}<br>
                    {/foreach}
                {else}
                    {t}(none){/t}
                {/if}
                </td>
            </tr>
        </tbody>
    </table>
    {/foreach}
    </div>
</div>
{elseif !isset($empty_schoolmgr_groups_now_phase2_done)} <!-- Phase2 flow control -->
<div class="row">
    <div class="col s12">
    <h3>{t}Step 2:{/t} {t}Empty SchoolManager administered object groups{/t}:</h3>

    <p>
        {t}The following list shows all parent groups managed by SchoolManager. You are about to empty all listed groups, i.e.: drop all members from those groups.{/t}
    </p>

    <p>
        {t}With the next import of students (and their parents), many of these parent groups will get automatically re-populated with members. At the beginning of a new school year, you want to do this!{/t}
    </p>

    <p>
        {t}Flush all members from the listed parent (object) groups now? After you have clicked this button, the process cannot be reverted! Emptying all groups may take some time, so please be patient while waiting…{/t}
    </p>

    <button class="btn-small primary" name="empty_schoolmgr_groups_now_phase3" type="submit">{t}Really empty all parent (object) groups listed below, now!{/t}</button>

    <hr class="divider">

    {foreach from=$data_ogroups item=group key=key}
    <table summary="{t 1=$group['cn'][0]}Group object: %1{/t}">
        <thead>
            <tr>
            <th style="width: 20%;">
                {t}Group Name{/t}
            </th>
            <th style="width: 30%;">
                {t}Description{/t}
            </th>
            <th>
                {t}Members (to be removed){/t}
            </th>
            </tr>
        </thead>
        <tbody>
            <tr>
                <td>{$group['cn'][0]}</td>
                <td>{$group['description'][0]}</td>
                <td>
                {if isset($group['member']) && (count($group['member']) > 0)}
                    {foreach from=$group['member'] item=value key=key}
                        {$value}<br>
                    {/foreach}
                {else}
                    {t}(none){/t}
                {/if}
                </td>
            </tr>
        </tbody>
    </table>
    {/foreach}
    </div>
</div>
{elseif !isset($empty_schoolmgr_groups_now_phase3_done)} <!-- Phase3 flow control -->
<div class="row">
    <div class="col s12">

    <h3>{t}Step 3:{/t} {t}Statistics{/t}:</h3>

    <p>
        {t}The clean-up process of SchoolManager-managed groups has been complete successfully, now. SchoolManager is ready for importing a new school year, now.{/t}
    </p>

    <ul>
        <li>{t 1=$cleanup_stats['classes'] 2=$cleanup_stats['classes_empty']}Emptied class groups: %1, already empty: %2{/t}
        <li>{t 1=$cleanup_stats['courses'] 2=$cleanup_stats['courses_empty']}Emptied course groups: %1, already empty: %2{/t}
        <li>{t 1=$cleanup_stats['subjects'] 2=$cleanup_stats['subjects_empty']}Emptied subject groups: %1, already empty: %2{/t}
        <li>{t 1=$cleanup_stats['parents'] 2=$cleanup_stats['parents_empty']}Emptied parent groups: %1, already empty: %2{/t}
    </ul>

    {if ($cleanup_stats['unmanaged_groups'] + $cleanup_stats['unmanaged_groups_empty'] + $cleanup_stats['unmanaged_ogroups'] + $cleanup_stats['unmanaged_ogroups_empty'] > 0)}
        <p>
            {t}Some groups were found in the SchoolManager area (OU) that should be moved out of there, please introspect manually. Let this be done by someone with LDAP expertise.{/t}
        </p>

        <ul>
            <li>{t 1=$cleanup_stats['unmanaged_groups'] 2=$cleanup_stats['unmanaged_groups_empty']}Ignored POSIX groups: %1, already empty: %2{/t}
            <li>{t 1=$cleanup_stats['unmanaged_ogroups'] 2=$cleanup_stats['unmanaged_ogroups_empty']}Ignored object groups: %1, already empty: %2{/t}
        </ul>
    {/if}

    <p>
        {t escape=no}You can continue with importing teacher accounts now. You can use the &quot;Import Teachers&quot; SchoolManager module for this.{/t}
    </p>
    </div>
</div>
{/if} <!-- Phase3 flow control -->
</div>
