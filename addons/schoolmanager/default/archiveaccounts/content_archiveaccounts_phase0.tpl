<input type="hidden" name="phase_00">

<h3>{t 1="1/11"}STEP %1: Choose archive type{/t}</h3>
<p>
    {t}Choose archive type. teacher and students are seperate. The students home directory will be deleted directly.{/t}
    {t}The teachers home directory will be removed after a period of time.{/t}
</p>

<label for="archivetype_id">{t}Type of user to archive{/t}</label>
{foreach $available_archive_types as $id=>$archive_type}
<label>
<input type="radio" name="archivetype_id" value="{$id}"{if $id == $preset_archive_type_id} checked{/if}>
<span>{$archive_type}</span>
</label>
{/foreach}
